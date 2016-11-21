module calculate

    use, intrinsic :: ieee_arithmetic, only: ieee_value, ieee_quiet_nan
    use, intrinsic :: iso_c_binding
    implicit none
    private

    public :: queryConstants, queryOperators, queryFunctions, Expression

    integer, parameter :: MAX_CHARS = 4096
    integer, parameter :: ERROR_CHARS = 64
    character(len=7), parameter :: ERROR_FMT = '(4096A)'


    type :: Expression
        type(c_ptr), private :: handler = c_null_ptr
    contains
        final :: freeExpression
        procedure, non_overridable :: clear => clearExpression
        procedure, non_overridable :: check => checkExpression
        procedure, non_overridable :: expression => getExpression
        procedure, non_overridable :: variables => getVariables
        procedure, non_overridable :: infix => getInfix
        procedure, non_overridable :: postfix => getPostfix
        procedure, non_overridable :: tree => getTree
        procedure, non_overridable :: eval => evaluateArray
        procedure, non_overridable :: assign => assignExpression
        generic :: assignment(=) => assign
    end type

    interface Expression
        module procedure createNewExpression
    end interface


    type, bind(c) :: LibraryTemplate
        type(c_funptr) :: queryConstants
        type(c_funptr) :: queryOperators
        type(c_funptr) :: queryFunctions
        type(c_funptr) :: createExpression
        type(c_funptr) :: newExpression
        type(c_funptr) :: freeExpression
        type(c_funptr) :: getExpression
        type(c_funptr) :: getVariables
        type(c_funptr) :: getInfix
        type(c_funptr) :: getPostfix
        type(c_funptr) :: getTree
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
        subroutine queryWrapper(query) bind(c)
            import :: c_char
            character(kind=c_char, len=1), dimension(*) :: query
        end subroutine

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

        subroutine getWrapper(expr, get) bind(c)
            import :: c_ptr, c_char
            type(c_ptr), value :: expr
            character(kind=c_char, len=1), dimension(*) :: get
        end subroutine

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

    function queryString(input, this) result (output)
        character(len=*), intent(in) :: input
        class(Expression), intent(in), optional :: this
        character(len=:), allocatable :: output

        type(LibraryTemplate), pointer :: Calculate
        character(kind=c_char, len=1), dimension(MAX_CHARS) :: coutput

        procedure(queryWrapper), pointer :: query
        procedure(getWrapper), pointer :: get
        call c_f_pointer(getLibraryReference(), Calculate)

        call c_f_procpointer(Calculate%queryConstants, query)
        call c_f_procpointer(Calculate%getExpression, get)
        select case (input)
            case ("constants")
                call c_f_procpointer(Calculate%queryConstants, query)
            case ("operators")
                call c_f_procpointer(Calculate%queryOperators, query)
            case ("functions")
                call c_f_procpointer(Calculate%queryFunctions, query)
            case ("expression")
                call c_f_procpointer(Calculate%getExpression, get)
            case ("variables")
                call c_f_procpointer(Calculate%getVariables, get)
            case ("infix")
                call c_f_procpointer(Calculate%getInfix, get)
            case ("postfix")
                call c_f_procpointer(Calculate%getPostfix, get)
            case ("tree")
                call c_f_procpointer(Calculate%getTree, get)
        end select

        if (.not. present(this)) then
            call query(coutput)
        else
            call get(this%handler, coutput)
        end if

        output = fromChars(coutput)
    end function


    function queryConstants() result (constants)
        character(len=:), allocatable :: constants

        constants = queryString('constants')
    end function

    function queryOperators() result (operators)
        character(len=:), allocatable :: operators

        operators = queryString('operators')
    end function

    function queryFunctions() result (functions)
        character(len=:), allocatable :: functions

        functions = queryString('functions')
    end function


    function createNewExpression(expr, vars, error) result (this)
        character(len=*), intent(in) :: expr
        character(len=*), intent(in), optional :: vars
        character(len=*), intent(out), optional :: error
        type(Expression) :: this

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
    end function

    subroutine assignExpression(this, other)
        class(Expression), intent(inout) :: this
        type(Expression), intent(in) :: other

        type(LibraryTemplate), pointer :: Calculate
        procedure(newExpressionWrapper), pointer :: new

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%newExpression, new)

        call freeExpression(this)
        this%handler = new( &
            toChars(other%expression()), &
            toChars(other%variables()) &
        )
    end subroutine

    subroutine freeExpression(this)
        type(Expression), intent(inout) :: this

        type(LibraryTemplate), pointer :: Calculate
        procedure(freeExpressionWrapper), pointer :: free

        call c_f_pointer(getLibraryReference(), Calculate)
        call c_f_procpointer(Calculate%freeExpression, free)

        call free(this%handler)
        this%handler = c_null_ptr
    end subroutine


    subroutine clearExpression(this)
        class(Expression), intent(inout) :: this

        call freeExpression(this)
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

        expr = queryString('expression', this)
    end function

    function getVariables(this) result (vars)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: vars

        vars = queryString('variables', this)
    end function

    function getInfix(this) result (infix)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: infix

        infix = queryString('infix', this)
    end function

    function getPostfix(this) result (postfix)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: postfix

        postfix = queryString('postfix', this)
    end function

    function getTree(this) result (tree)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: tree

        tree = queryString('tree', this)
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
    end function

end module
