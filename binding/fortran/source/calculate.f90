module calculate_fortran

    use, intrinsic :: iso_c_binding, only: c_ptr, c_null_ptr
    implicit none
    private

    public :: calculate, Expression

    integer, parameter :: MAX_CHARS = 4096
    integer, parameter :: ERROR_CHARS = 64
    character(len=7), parameter :: ERROR_FMT = '(4096A)'


    type :: CalculateLibrary
    contains
        procedure, nopass :: version => version
        procedure, nopass :: author => author
        procedure, nopass :: date => date
        procedure, nopass :: constants => constants
        procedure, nopass :: operators => operators
        procedure, nopass :: functions => functions
        procedure, nopass :: Expression => construct
    end type
    type(CalculateLibrary) :: calculate


    type :: Expression
        type(c_ptr), private :: handler = c_null_ptr
    contains
        final :: free
        procedure, non_overridable :: clear => clear
        procedure, non_overridable :: check => check
        procedure, non_overridable :: expression => string
        procedure, non_overridable :: variables => variables
        procedure, non_overridable :: infix => infix
        procedure, non_overridable :: postfix => postfix
        procedure, non_overridable :: tree => tree
        procedure, non_overridable :: eval => evaluate
        procedure, non_overridable :: assign => assign
        generic :: assignment(=) => assign
    end type

    interface Expression
        module procedure construct
    end interface


    interface
        module function version()
            character(len=:), allocatable :: version
        end function

        module function author()
            character(len=:), allocatable :: author
        end function

        module function date()
            character(len=:), allocatable :: date
        end function


        module function constants()
            character(len=:), allocatable :: constants
        end function

        module function operators()
            character(len=:), allocatable :: operators
        end function

        module function functions()
            character(len=:), allocatable :: functions
        end function


        module function construct(expr, vars, error) result (this)
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

        module function check(this)
            class(Expression), intent(in) :: this
            logical :: check
        end function

        module function string(this)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: string
        end function

        module function variables(this)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: variables
        end function

        module function infix(this)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: infix
        end function

        module function postfix(this)
            class(Expression), intent(in) :: this
            character(len=:), allocatable :: postfix
        end function

        module function tree(this)
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
