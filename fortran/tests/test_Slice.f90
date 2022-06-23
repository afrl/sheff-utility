    program test_Slice
    ! Tests linkage to utility model

    use iso_c_binding
    use utility

    implicit none

    real(C_DOUBLE), dimension(10) :: arr
    type(SliceDouble) :: arr_slice

    arr(:) = 0.
    arr_slice = slice(arr)
    ! TODO finish wrapping Slice methods
    !print *, 'size = ', arr_slice%size()

    end program test_Slice