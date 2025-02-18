import { useQuery } from "react-query";
import { useSelector } from "react-redux";
import { selectActiveUser } from "../../features/user/userSelector";
import axiosService from "../../services/axiosService";
import { BookingCalendarType } from "../../typings/bookingType";

const useUserHasActiveBooking = () => {
  const user = useSelector(selectActiveUser);

  return useQuery<BookingCalendarType | null>(
    ["activeBookingQuery"],
    async () => {
      const response = await axiosService.get<BookingCalendarType>(
        `Bookings/${user.id}/hasBooked`
      );
      if (response.status === 200) return response.data;
      else return null;
    },
    { enabled: user.id != "" }
  );
};

export default useUserHasActiveBooking;
