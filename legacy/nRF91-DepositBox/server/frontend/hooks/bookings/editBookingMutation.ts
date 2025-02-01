import { useMutation } from "react-query";
import { useSelector } from "react-redux";
import { selectActiveBooking } from "../../features/booking/bookingSelector";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";
import { BookingVMType } from "../../typings/bookingType";

const useUpdateBooking = () => {
  const activeBooking = useSelector(selectActiveBooking);

  if (!activeBooking) {
    throw new Error("No active booking");
  }

  return useMutation({
    mutationKey: "updateBookingMutation",
    mutationFn: (data: BookingVMType) =>
      axiosService.put(`/Bookings/${activeBooking.bookingID}`, data),
    onSettled: () => {
      queryClient.invalidateQueries("bookingSpanQuery");
      queryClient.invalidateQueries("activeBookingQuery");
      queryClient.invalidateQueries("bookingsQuery");
    },
  });
};

export default useUpdateBooking;
