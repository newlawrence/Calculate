module calculate

    use, intrinsic :: iso_c_binding, only: c_ptr, c_null_ptr
    implicit none
    private

    public :: Expression, freeExpression

    integer, parameter :: MAX_CHARS = 8192
    integer, parameter :: ERROR_CHARS = 64
    character(len=7), parameter :: ERROR_FMT = '(8192A)'


    type :: Expression
        type(c_ptr), private :: handler = c_null_ptr
    contains
        final :: freeExpression
        procedure, non_overridable :: check => checkExpression
        procedure, non_overridable :: expression => getExpression
        procedure, non_overridable :: variables => getVariables
        procedure, non_overridable :: eval => evaluateArray
        procedure, non_overridable :: assign => assignExpression
        generic :: assignment(=) => assign
    end type

    interface Expression
        module procedure createNewExpression
    end interface


    interface
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

        module function evaluateArray(this, args, error) result (result)
            class(Expression), intent(in) :: this
            real(kind=8), dimension(:), intent(in), optional :: args
            character(len=*), intent(out), optional :: error
            real(kind=8) :: result
        end function
    end interface

end module
