submodule (calculate) calculate_wrapper

    use, intrinsic :: ieee_arithmetic, only: ieee_value, ieee_quiet_nan
    use, intrinsic :: iso_c_binding
    implicit none


    type, bind(c) :: LibraryTemplate
        type(c_funptr) :: createExpression
        type(c_funptr) :: newExpression
        type(c_funptr) :: freeExpression
        type(c_funptr) :: getExpression
        type(c_funptr) :: getVariables
        type(c_funptr) :: evaluateArray
        type(c_funptr) :: evalArray
        type(c_funptr) :: eval
    end type

    interface
        function getLibraryReference() bind(c, name='get_calculate_reference')
            import :: c_ptr
            type(c_ptr) :: getLibraryReference
        end function
    end interface

    abstract interface
        function createExpressionWrapper(expr, vars, error) bind(c)
            import :: c_ptr, c_char
            character(kind=c_char, len=1), dimension(*) :: expr
            character(kind=c_char, len=1), dimension(*) :: vars
            character(kind=c_char, len=1), dimension(*) :: error
            type(c_ptr) :: createExpressionWrapper
        end function

        function newExpressionWrapper(expr, vars) bind(c)
            import :: c_ptr, c_char
            character(kind=c_char, len=1), dimension(*) :: expr
            character(kind=c_char, len=1), dimension(*) :: vars
            type(c_ptr) :: newExpressionWrapper
        end function

        subroutine freeExpressionWrapper(expr) bind(c)
            import :: c_ptr
            type(c_ptr), value :: expr
        end subroutine

        function getExpressionWrapper(expr) bind(c)
            import :: c_ptr
            type(c_ptr), value :: expr
            type(c_ptr) :: getExpressionWrapper
        end function

        function getVariablesWrapper(expr) bind(c)
            import :: c_ptr
            type(c_ptr), value :: expr
            type(c_ptr) :: getVariablesWrapper
        end function

        function evaluateArrayWrapper(expr, args, size, error) bind(c)
            import :: c_ptr, c_char, c_int, c_double
            type(c_ptr), value :: expr
            real(kind=c_double), dimension(*) :: args
            integer(kind=c_int), value :: size
            character(kind=c_char, len=1), dimension(*) :: error
            real(kind=c_double) :: evaluateArrayWrapper
        end function
    end interface


contains

    function toChars(forstr) result (cstr)
        character(len=*), intent(in) :: forstr
        character(kind=c_char, len=1), dimension(len(forstr)+1) :: cstr

        integer :: c

        cstr = [(forstr(c : c), c=1, len(forstr))]
        cstr(len(forstr) + 1) = c_null_char
    end function

    function fromChars(cstr) result (forstr)
        character(kind=c_char, len=1), dimension(*), intent(in) :: cstr
        character(len=:), allocatable :: forstr

        integer :: c

        do c = 1, MAX_CHARS
            if (cstr(c) == c_null_char) exit
        end do
        c = c - 1

        allocate(character(len=c) :: forstr)
        do c = 1, len(forstr)
            forstr(c : c) = cstr(c)
        end do
    end function

    function fromPointer(cptr) result (forstr)
        type(c_ptr), intent(in) :: cptr
        character(len=:), allocatable :: forstr

        character, pointer, dimension(:) :: strptr
        integer :: c

        call c_f_pointer(cptr, strptr, [MAX_CHARS])
        do c = 1, MAX_CHARS
            if (strptr(c) == c_null_char) exit
        end do
        c = c - 1

        allocate(character(len=c) :: forstr)
        forstr = transfer(strptr(1 : c), forstr)
    end function


    module procedure createNewExpression
        type(LibraryTemplate), pointer :: Calculate
        procedure(createExpressionWrapper), pointer :: create
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        character(len=:), allocatable :: message
        integer :: c

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%createExpression, create)

        if (present(vars)) then
            this%handler = create(toChars(expr), toChars(vars), cerror)
        else
            this%handler = create(toChars(expr), toChars(''), cerror)
        end if
        message = fromChars(cerror)

        if (present(error)) then
            if (len(message) > len(error)) then
                write (error, ERROR_FMT) ('*', c=1, len(error))
            else
                error = message
            end if
        end if
    end procedure

    module procedure assignExpression
        type(LibraryTemplate), pointer :: Calculate
        procedure(newExpressionWrapper), pointer :: new

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%newExpression, new)

        call freeExpression(this)
        this%handler = new( &
            toChars(other%expression()), &
            toChars(other%variables()) &
        )
    end procedure

    module procedure freeExpression
        type(LibraryTemplate), pointer :: Calculate
        procedure(freeExpressionWrapper), pointer :: free

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%freeExpression, free)

        call free(this%handler)
        this%handler = c_null_ptr
    end procedure


    module procedure clearExpression
        call freeExpression(this)
    end procedure

    module procedure checkExpression
        check = .false.
        if (len(getExpression(this)) > 0) check = .true.
    end procedure

    module procedure getExpression
        type(LibraryTemplate), pointer :: Calculate
        procedure(getExpressionWrapper), pointer :: get

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%getExpression, get)

        expr = fromPointer(get(this%handler))
    end procedure

    module procedure getVariables
        type(LibraryTemplate), pointer :: Calculate
        procedure(getVariablesWrapper), pointer :: get

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%getVariables, get)

        vars = fromPointer(get(this%handler))
    end procedure

    module procedure evaluateArray
        type(LibraryTemplate), pointer :: Calculate
        procedure(evaluateArrayWrapper), pointer :: eval
        real(kind=8), dimension(1) :: default
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        character(len=:), allocatable :: message
        integer :: c

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%evaluateArray, eval)

        if (present(args)) then
            result = eval(this%handler, args, size(args), cerror)
        else
            result = eval(this%handler, default, 0, cerror)
        end if
        message = fromChars(cerror)

        if (present(error)) then
            if (len(message) > len(error)) then
                write (error, ERROR_FMT) ('*', c=1, len(error))
            else
                error = message
            end if
        end if
    end procedure

end submodule
