import { useMutation } from "react-query";
import { useSelector } from "react-redux";
import { selectSelectedBoat } from "../../features/boat/boatSelector";
import { selectActiveBooking } from "../../features/booking/bookingSelector";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";

const useCancelBookingMutation = () => {
  const activeBooking = useSelector(selectActiveBooking);
  const boat = useSelector(selectSelectedBoat);

  if (!activeBooking) {
    throw new Error("No active booking");
  }

  return useMutation({
    mutationKey: "cancelBookingMutation",
    mutationFn: () =>
      axiosService.put(`/Bookings/${activeBooking.bookingID}`, {
        startTime: activeBooking.startTime,
        endTime: activeBooking.endTime,
        status: "C",
        comment: activeBooking!.comment,
        userID: activeBooking!.bookingOwnerId,
        boatID: boat.boatID === 0 ? 1 : boat.boatID,
        isOfficial: activeBooking!.isOfficial,
      }),
    onSettled: () => {
      queryClient.invalidateQueries("bookingSpanQuery");
      queryClient.invalidateQueries("activeBookingQuery");
      queryClient.invalidateQueries("bookingsQuery");
    },
  });
};

export default useCancelBookingMutation;
