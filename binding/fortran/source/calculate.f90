module calculate

    use, intrinsic :: ieee_arithmetic, only: ieee_value, ieee_quiet_nan
    use, intrinsic :: iso_c_binding
    implicit none
    private

    public :: Expression, freeExpression

    integer(kind=8), parameter :: MAGIC_NUMBER = 103592
    integer, parameter :: MAX_CHARS = 8192
    integer, parameter :: ERROR_CHARS = 64
    character(len=7), parameter :: ERROR_FMT = '(8192A)'


    type :: Expression
        integer(kind=8), private :: init_number = MAGIC_NUMBER
        type(c_ptr), private :: handler = c_null_ptr

    contains
        final :: freeExpression

        procedure, non_overridable :: check => checkExpression
        procedure, non_overridable :: expression => getExpression
        procedure, non_overridable :: variables => getVariables
        procedure, non_overridable :: eval => evaluateArray

        procedure, non_overridable :: assign => assignExpression
        procedure, non_overridable :: assignNew => assignNewExpression
        generic :: assignment(=) => assign, assignNew
    end type


    interface Expression
        module procedure createNewExpression
    end interface

    type :: NewExpression
        character(len=:), allocatable, private :: expr
        character(len=:), allocatable, private :: vars
    end type


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
        function libraryReference() bind(c, name='calculateReference')
            import :: c_ptr
            type(c_ptr) :: libraryReference
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


    function createNewExpression(expr, vars, error) result (this)
        character(len=*), intent(in) :: expr
        character(len=*), intent(in), optional :: vars
        character(len=*), intent(out), optional :: error
        type(NewExpression) :: this

        type(LibraryTemplate), pointer :: Calculate
        procedure(createExpressionWrapper), pointer :: create
        procedure(freeExpressionWrapper), pointer :: free
        type(c_ptr) :: handler
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        character(len=:), allocatable :: message
        integer :: c

        call c_f_pointer(libraryReference(), Calculate)
        call c_f_procpointer(Calculate%createExpression, create)
        call c_f_procpointer(Calculate%freeExpression, free)

        if (present(vars)) then
            handler = create(toChars(expr), toChars(vars), cerror)
        else
            handler = create(toChars(expr), toChars(''), cerror)
        end if
        message = fromChars(cerror)

        this%expr = expr
        this%vars = vars
        call free(handler)

        if (present(error)) then
            if (len(message) > len(error)) then
                write (error, ERROR_FMT) ('*', c=1, len(error))
            else
                error = message
            end if
        end if
    end function

    subroutine assignExpression(this, other)
        class(Expression), intent(inout) :: this
        type(Expression), intent(in) :: other

        type(LibraryTemplate), pointer :: Calculate
        procedure(newExpressionWrapper), pointer :: new

        if (other%init_number == MAGIC_NUMBER) then
            call c_f_pointer(libraryReference(), Calculate)
            call c_f_procpointer(Calculate%newExpression, new)

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
    end subroutine

    subroutine assignNewExpression(this, other)
        class(Expression), intent(inout) :: this
        type(NewExpression), intent(in) :: other

        type(LibraryTemplate), pointer :: Calculate
        procedure(newExpressionWrapper), pointer :: new

        call c_f_pointer(libraryReference(), Calculate)
        call c_f_procpointer(Calculate%newExpression, new)

        if (this%init_number == MAGIC_NUMBER) call freeExpression(this)
        this%init_number = MAGIC_NUMBER
        this%handler = new(toChars(other%expr), toChars(other%vars))
    end subroutine


    subroutine freeExpression(this)
        type(Expression), intent(inout) :: this

        type(LibraryTemplate), pointer :: Calculate
        procedure(freeExpressionWrapper), pointer :: free

        if (this%init_number == MAGIC_NUMBER) then
            call c_f_pointer(libraryReference(), Calculate)
            call c_f_procpointer(Calculate%freeExpression, free)

            call free(this%handler)
            this%handler = c_null_ptr
        end if
    end subroutine

    function checkExpression(this) result (check)
        class(Expression), intent(in) :: this
        logical :: check

        check = .false.
        if (len(getExpression(this)) > 0) check = .true.
    end function

    function getExpression(this) result (expr)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: expr

        type(LibraryTemplate), pointer :: Calculate
        procedure(getExpressionWrapper), pointer :: get

        expr = ''
        if (this%init_number == MAGIC_NUMBER) then
            call c_f_pointer(libraryReference(), Calculate)
            call c_f_procpointer(Calculate%getExpression, get)

            expr = fromPointer(get(this%handler))
        end if
    end function

    function getVariables(this) result (vars)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: vars

        type(LibraryTemplate), pointer :: Calculate
        procedure(getVariablesWrapper), pointer :: get

        vars = ''
        if (this%init_number == MAGIC_NUMBER) then
            call c_f_pointer(libraryReference(), Calculate)
            call c_f_procpointer(Calculate%getVariables, get)

            vars = fromPointer(get(this%handler))
        end if
    end function

    function evaluateArray(this, args, error) result (result)
        class(Expression), intent(in) :: this
        real(kind=8), dimension(:), intent(in), optional :: args
        character(len=*), intent(out), optional :: error
        real(kind=8) :: result

        type(LibraryTemplate), pointer :: Calculate
        procedure(evaluateArrayWrapper), pointer :: eval
        real(kind=8), dimension(1) :: default
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        character(len=:), allocatable :: message
        integer :: c

        if (this%init_number == MAGIC_NUMBER) then
            call c_f_pointer(libraryReference(), Calculate)
            call c_f_procpointer(Calculate%evaluateArray, eval)

            if (present(args)) then
                result = eval(this%handler, args, size(args), cerror)
            else
                result = eval(this%handler, default, 0, cerror)
            end if
            message = fromChars(cerror)
        else
            message = 'Not initialized'
            result = ieee_value(result, ieee_quiet_nan)
        end if

        if (present(error)) then
            if (len(message) > len(error)) then
                write (error, ERROR_FMT) ('*', c=1, len(error))
            else
                error = message
            end if
        end if
    end function

end module
