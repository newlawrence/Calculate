program calcfor

    use calculate
    implicit none

    type(Expression) :: expr
    character(len=256) :: vars
    character(len=64) :: error
    real(kind=8), dimension(:), allocatable :: vals
    real(kind=8) :: result

    character(len=32) :: arg
    integer :: count
    integer :: i


    count = command_argument_count()
    if (mod(count, 2) == 1) then
        vars = ''
        do i = 2, count, 2
            call get_command_argument(i, arg)
            write(vars, '(3A)') trim(vars), ',', trim(arg)
        end do
        vars = vars(2 :)
        call get_command_argument(1, arg)
        expr = Expression(arg, vars=vars, error=error)

        if (expr%check()) then
            allocate (vals((count - 1) / 2))
            do i = 3, count, 2
                call get_command_argument(i, arg)
                read(arg, *) vals(i - (i + 1) / 2)
            end do

            result = expr%eval(vals, error=error)
            if (len(trim(error)) == 0) then
                print '(ES14.6E3)', result
            else
                print '(A)', error
            end if

            deallocate (vals)
            call freeExpression(expr)
        else
            print '(A)', trim(error)
        end if
    end if

end program
