module calculate

    use, intrinsic :: iso_c_binding, only: c_ptr, c_null_ptr
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


    interface
        module function queryConstants() result (constants)
            character(len=:), allocatable :: constants
        end function

        module function queryOperators() result (operators)
            character(len=:), allocatable :: operators
        end function

        module function queryFunctions() result (functions)
            character(len=:), allocatable :: functions
        end function


        module function createNewExpression(expr, vars, error) result (this)
            character(len=*), intent(in) :: expr
            character(len=*), intent(in), optional :: vars
            character(len=*), intent(out), optional :: error
            type(Expression) :: this
        end function

        module subroutine assignExpression(this, other)
            class(Expression), intent(inout) :: this
            type(Expression), intent(in) :: other
        end subroutine

        module subroutine freeExpression(this)
            type(Expression), intent(inout) :: this
        end subroutine


        module subroutine clearExpression(this)
            class(Expression), intent(inout) :: this
        end subroutine

        module function checkExpression(this) result (check)
            class(Expression), intent(in) :: this
            logical :: check
        end function

        module function getExpression(this) result (expr)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: expr
        end function

        module function getVariables(this) result (vars)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: vars
        end function

        module function getInfix(this) result (infix)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: infix
        end function

        module function getPostfix(this) result (postfix)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: postfix
        end function

        module function getTree(this) result (tree)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: tree
        end function

        module function evaluateArray(this, args, error) result (result)
            class(Expression), intent(in) :: this
            real(kind=8), dimension(:), intent(in), optional :: args
            character(len=*), intent(out), optional :: error
            real(kind=8) :: result
        end function
    end interface

end module
