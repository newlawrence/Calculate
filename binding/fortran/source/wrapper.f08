submodule (calculate) calculate_wrapper

    use, intrinsic :: ieee_arithmetic, only: ieee_value, ieee_quiet_nan
    implicit none


    type, bind(c) :: LibraryTemplate
        type(c_funptr) :: createExpression
        type(c_funptr) :: newExpression
        type(c_funptr) :: freeExpression
        type(c_funptr) :: compare
        type(c_funptr) :: getExpression
        type(c_funptr) :: getVariables
        type(c_funptr) :: evaluateArray
        type(c_funptr) :: evalArray
        type(c_funptr) :: eval  
    end type

    type(LibraryTemplate), bind(c, name='Calculate') :: CalculateLibrary


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

        function compareWrapper(one, another) bind(c)
            import :: c_ptr, c_int
            type(c_ptr), value :: one
            type(c_ptr), value :: another
            integer(kind=c_int) :: compareWrapper
        end function

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


    module procedure createRVExpression
        procedure(createExpressionWrapper), pointer :: create
        procedure(freeExpressionWrapper), pointer :: free
        type(c_ptr) :: handler
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        integer :: c

        call c_f_procpointer(CalculateLibrary%createExpression, create)
        call c_f_procpointer(CalculateLibrary%freeExpression, free)
        if (present(vars)) then
            handler = create(toChars(expr), toChars(vars), cerror)
        else
            handler = create(toChars(expr), toChars(''), cerror)
        end if
        this%error = fromChars(cerror)

        this%expr = expr
        this%vars = vars
        call free(handler)

        if (present(error)) then
            if (len(this%error) > len(error)) then
                write (error, '(999999A)') ('*', c=1, len(error))
            else
                error = this%error
            end if
        end if
    end procedure

    module procedure assignExpression
        procedure(newExpressionWrapper), pointer :: new

        if (other%init_number == MAGIC_NUMBER) then
            call c_f_procpointer(CalculateLibrary%newExpression, new)

            if (this%init_number == MAGIC_NUMBER) call freeExpression(this)
            this%init_number = MAGIC_NUMBER
            this%handler = new( &
                toChars(other%expression()), &
                toChars(other%variables()) &
            )
        else
            this%init_number = MAGIC_NUMBER
            this%handler = c_null_ptr
        end if
    end procedure

    module procedure assignRVExpression
        procedure(newExpressionWrapper), pointer :: new

        call c_f_procpointer(CalculateLibrary%newExpression, new)
        if (this%init_number == MAGIC_NUMBER) call freeExpression(this)
        this%init_number = MAGIC_NUMBER
        this%handler = new(toChars(other%expr), toChars(other%vars))
    end procedure

    module procedure compareExpression
        integer :: comparison
        procedure(compareWrapper), pointer :: compare

        comp = .false.
        if ( &
            this%init_number == MAGIC_NUMBER .and. &
            other%init_number == MAGIC_NUMBER &
        ) then
            if (this%init_number == MAGIC_NUMBER) then
                call c_f_procpointer(CalculateLibrary%compare, compare)

                comparison = compare(this%handler, other%handler)
                if (comparison > 0) comp = .true.
            end if
        end if
    end procedure

    module procedure compareRVExpression
        integer :: comparison
        type(Expression) :: another
        procedure(compareWrapper), pointer :: compare

        comp = .false.
        if (this%init_number == MAGIC_NUMBER) then
            call c_f_procpointer(CalculateLibrary%compare, compare)
            another = other

            comparison = compare(this%handler, another%handler)
            call freeExpression(another)
            if (comparison > 0) comp = .true.
        end if
    end procedure

    module procedure compareRVExpressions
        integer :: comparison
        type(Expression) :: one
        type(Expression) :: another
        procedure(compareWrapper), pointer :: compare

        comp = .false.
        call c_f_procpointer(CalculateLibrary%compare, compare)
        one = this
        another = other

        comparison = compare(one%handler, another%handler)
        call freeExpression(another)
        call freeExpression(one)
        if (comparison > 0) comp = .true.
    end procedure


    module procedure freeExpression
        procedure(freeExpressionWrapper), pointer :: free

        if (this%init_number == MAGIC_NUMBER) then
            call c_f_procpointer(CalculateLibrary%freeExpression, free)

            call free(this%handler)
            this%handler = c_null_ptr
        end if
    end procedure

    module procedure checkExpression
        check = .false.
        if (len(getExpression(this)) > 0) check = .true.
    end procedure

    module procedure getExpressionFixed
        procedure(getExpressionWrapper), pointer :: get

        expr = ""
        if (this%init_number == MAGIC_NUMBER) then
            call c_f_procpointer(CalculateLibrary%getExpression, get)

            expr = fromPointer(get(this%handler))
        end if
    end procedure

    module procedure getVariablesFixed
        procedure(getVariablesWrapper), pointer :: get

        vars = ""
        if (this%init_number == MAGIC_NUMBER) then
            call c_f_procpointer(CalculateLibrary%getVariables, get)

            vars = fromPointer(get(this%handler))
        end if
    end procedure

    module procedure evaluateArray
        procedure(evaluateArrayWrapper), pointer :: eval
        real(kind=8), dimension(1) :: default
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        character(len=:), allocatable :: message
        integer :: c

        if (this%init_number == MAGIC_NUMBER) then
            call c_f_procpointer(CalculateLibrary%evaluateArray, eval)

            if (present(args)) then
                result = eval(this%handler, args, size(args), cerror)
            else
                result = eval(this%handler, default, 0, cerror)
            end if
            message = fromChars(cerror)
        else
            message = "Not initialized"
            result = ieee_value(result, ieee_quiet_nan)
        end if

        if (present(error)) then
            if (len(message) > len(error)) then
                write (error, '(999999A)') ('*', c=1, len(error))
            else
                error = message
            end if
        end if
    end procedure

end submodule
