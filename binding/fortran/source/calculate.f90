module calculate

    use, intrinsic :: iso_c_binding, only: c_ptr, c_null_ptr
    implicit none
    private

    public :: constants, operators, functions, Expression

    integer, parameter :: MAX_CHARS = 4096
    integer, parameter :: ERROR_CHARS = 64
    character(len=7), parameter :: ERROR_FMT = '(4096A)'


    type :: Expression
        type(c_ptr), private :: handler = c_null_ptr
    contains
        final :: free
        procedure, non_overridable :: clear => clear
        procedure, non_overridable :: check => check
        procedure, non_overridable :: expression => expression
        procedure, non_overridable :: variables => variables
        procedure, non_overridable :: infix => infix
        procedure, non_overridable :: postfix => postfix
        procedure, non_overridable :: tree => tree
        procedure, non_overridable :: eval => evaluate
        procedure, non_overridable :: assign => assign
        generic :: assignment(=) => assign
    end type

    interface Expression
        module procedure createNewExpression
    end interface


    interface
        module function constants() result (constants)
            character(len=:), allocatable :: constants
        end function

        module function operators() result (operators)
            character(len=:), allocatable :: operators
        end function

        module function functions() result (functions)
            character(len=:), allocatable :: functions
        end function


        module function createNewExpression(expr, vars, error) result (this)
            character(len=*), intent(in) :: expr
            character(len=*), intent(in), optional :: vars
            character(len=*), intent(out), optional :: error
            type(Expression) :: this
        end function

        module subroutine assign(this, other)
            class(Expression), intent(inout) :: this
            type(Expression), intent(in) :: other
        end subroutine

        module subroutine free(this)
            type(Expression), intent(inout) :: this
        end subroutine

        module subroutine clear(this)
            class(Expression), intent(inout) :: this
        end subroutine

        module function check(this) result (check)
            class(Expression), intent(in) :: this
            logical :: check
        end function

        module function expression(this) result (expr)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: expr
        end function

        module function variables(this) result (vars)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: vars
        end function

        module function infix(this) result (infix)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: infix
        end function

        module function postfix(this) result (postfix)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: postfix
        end function

        module function tree(this) result (tree)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: tree
        end function

        module function evaluate(this, args, error) result (result)
            class(Expression), intent(in) :: this
            real(kind=8), dimension(:), intent(in), optional :: args
            character(len=*), intent(out), optional :: error
            real(kind=8) :: result
        end function
    end interface

end module
