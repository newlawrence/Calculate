submodule (calculate) calculate_wrapper

    use, intrinsic :: ieee_arithmetic, only: ieee_value, ieee_quiet_nan
    use, intrinsic :: iso_c_binding
    implicit none


    type, bind(c) :: LibraryTemplate
        type(c_funptr) :: constants
        type(c_funptr) :: operators
        type(c_funptr) :: functions
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

        type(LibraryTemplate), pointer :: calculate
        character(kind=c_char, len=1), dimension(MAX_CHARS) :: coutput

        procedure(queryWrapper), pointer :: query
        procedure(getWrapper), pointer :: get
        call c_f_pointer(getLibraryReference(), calculate)

        select case (input)
            case ("constants")
                call c_f_procpointer(calculate%constants, query)
            case ("operators")
                call c_f_procpointer(calculate%operators, query)
            case ("functions")
                call c_f_procpointer(calculate%functions, query)
            case ("expression")
                call c_f_procpointer(calculate%getExpression, get)
            case ("variables")
                call c_f_procpointer(calculate%getVariables, get)
            case ("infix")
                call c_f_procpointer(calculate%getInfix, get)
            case ("postfix")
                call c_f_procpointer(calculate%getPostfix, get)
            case ("tree")
                call c_f_procpointer(calculate%getTree, get)
        end select

        if (.not. present(this)) then
            call query(coutput)
        else
            call get(this%handler, coutput)
        end if

        output = fromChars(coutput)
    end function


    module procedure constants
        constants = queryString('constants')
    end procedure

    module procedure operators
        operators = queryString('operators')
    end procedure

    module procedure functions
        functions = queryString('functions')
    end procedure


    module procedure createNewExpression
        type(LibraryTemplate), pointer :: calculate
        procedure(createExpressionWrapper), pointer :: create
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        character(len=:), allocatable :: message
        integer :: c

        call c_f_pointer(getLibraryReference(), calculate)
        call c_f_procpointer(calculate%createExpression, create)

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
        type(LibraryTemplate), pointer :: calculate
        procedure(newExpressionWrapper), pointer :: new

        call c_f_pointer(getLibraryReference(), calculate)
        call c_f_procpointer(calculate%newExpression, new)

        call freeExpression(this)
        this%handler = new( &
            toChars(other%expression()), &
            toChars(other%variables()) &
        )
    end procedure

    module procedure freeExpression
        type(LibraryTemplate), pointer :: calculate
        procedure(freeExpressionWrapper), pointer :: free

        call c_f_pointer(getLibraryReference(), calculate)
        call c_f_procpointer(calculate%freeExpression, free)

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
        expr = queryString('expression', this)
    end procedure

    module procedure getVariables
        vars = queryString('variables', this)
    end procedure

    module procedure getInfix
        infix = queryString('infix', this)
    end procedure

    module procedure getPostfix
        postfix = queryString('postfix', this)
    end procedure

    module procedure getTree
        tree = queryString('tree', this)
    end procedure

    module procedure evaluateArray
        type(LibraryTemplate), pointer :: calculate
        procedure(evaluateArrayWrapper), pointer :: eval
        real(kind=8), dimension(1) :: default
        character(kind=c_char, len=1), dimension(ERROR_CHARS) :: cerror
        character(len=:), allocatable :: message
        integer :: c

        call c_f_pointer(getLibraryReference(), calculate)
        call c_f_procpointer(calculate%evaluateArray, eval)

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
