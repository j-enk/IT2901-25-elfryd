import { useQuery } from "react-query";
import { useSelector } from "react-redux";
import { selectSelectedBoat } from "../../features/boat/boatSelector";
import { selectFilterOnUser } from "../../features/booking/bookingSelector";
import { selectActiveUser } from "../../features/user/userSelector";
import axiosService from "../../services/axiosService";
import { BookingCalendarType } from "../../typings/bookingType";

const useBookings = (startDate: string, endDate: string) => {
  const isFilter = useSelector(selectFilterOnUser);
  const user = useSelector(selectActiveUser);
  const selectedBoat = useSelector(selectSelectedBoat);

  return useQuery<BookingCalendarType[]>(
    ["bookingSpanQuery", startDate, endDate, selectedBoat, isFilter],
    async () => {
      const boatID = selectedBoat.boatID === 0 ? 1 : selectedBoat.boatID;
      const URL = isFilter
        ? `Bookings/${boatID}/byUser/${user.id}`
        : `Bookings/${boatID}/span?startDate=${startDate}&endDate=${endDate}`;

      const response = await axiosService.get(URL);
      return response.data;
    },
    { enabled: startDate !== "" && endDate !== "" }
  );
};

export default useBookings;
