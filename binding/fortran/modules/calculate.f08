module calculate

    use, intrinsic :: iso_c_binding
    implicit none
    private

    public :: Expression, freeExpression, assignment(=), operator(==)

    integer(kind=8), parameter :: MAGIC_NUMBER = 103592
    integer, parameter :: MAX_CHARS = 8192
    integer, parameter :: ERROR_CHARS = 64


    type :: Expression
        integer(kind=8), private :: init_number = MAGIC_NUMBER
        type(c_ptr), private :: handler = c_null_ptr
    contains
        final :: freeExpression
        procedure, non_overridable :: check => checkExpression
        procedure, non_overridable :: expression => getExpression
        procedure, non_overridable :: variables => getVariables
        procedure, non_overridable :: eval => evaluateArray
    end type


    type :: RVExpression
        character(len=:), allocatable, private :: expr
        character(len=:), allocatable, private :: vars
        character(len=:), allocatable, private :: error
    end type


    interface Expression
        module procedure createRVExpression
    end interface

    interface assignment(=)
        module procedure assignExpression
        module procedure assignRVExpression
    end interface

    interface operator(==)
        module procedure compareExpression
        module procedure compareRVExpression
        module procedure compareRVExpressions
    end interface


    interface
        module function createRVExpression(expr, vars, error) result (this)
            character(len=*), intent(in) :: expr
            character(len=*), intent(in), optional :: vars
            character(len=*), intent(out), optional :: error
            type(RVExpression) :: this
        end function

        module subroutine assignExpression(this, other)
            class(Expression), intent(inout) :: this
            type(Expression), intent(in) :: other
            logical :: comp
        end subroutine

        module subroutine assignRVExpression(this, other)
            class(Expression), intent(inout) :: this
            type(RVExpression), intent(in) :: other
            logical :: comp
        end subroutine

        module function compareExpression(this, other) result (comp)
            class(Expression), intent(in) :: this
            type(Expression), intent(in) :: other
            logical :: comp
        end function

        module function compareRVExpression(this, other) result (comp)
            class(Expression), intent(in) :: this
            type(RVExpression), intent(in) :: other
            logical :: comp
        end function

        module function compareRVExpressions(this, other) result (comp)
            class(RVExpression), intent(in) :: this
            type(RVExpression), intent(in) :: other
            logical :: comp
        end function


        module subroutine freeExpression(this)
            type(Expression), intent(inout) :: this
        end subroutine

        module function checkExpression(this) result (check)
            class(Expression), intent(in) :: this
            logical :: check
        end function

        module function getExpressionFixed(this) result (expr)
            class(Expression), intent(in) :: this
            character(len=MAX_CHARS) :: expr
        end function

        module function getVariablesFixed(this) result (vars)
            class(Expression), intent(in) :: this
            character(len=MAX_CHARS) :: vars
        end function

        module function evaluateArray(this, args, error) result (result)
            class(Expression), intent(in) :: this
            real(kind=8), dimension(:), intent(in), optional :: args
            character(len=*), intent(out), optional :: error
            real(kind=8) :: result
        end function
    end interface


contains

    function getExpression(this) result (expr)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: expr

        expr = trim(getExpressionFixed(this))
    end function

    function getVariables(this) result (vars)
        class(Expression), intent(in) :: this
        character(len=:), allocatable :: vars

        vars = trim(getVariablesFixed(this))
    end function

end module
