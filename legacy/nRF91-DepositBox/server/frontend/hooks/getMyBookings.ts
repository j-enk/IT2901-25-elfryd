import { useQuery } from "react-query";
import axiosService from "../services/axiosService";
import { PaginationType } from "../typings/paginationType";
import { BookingCalendarType } from "../typings/bookingType";
import { BookingReadableType } from "../typings/bookingType";


export const getMyBookings = (
    userID: string,
) => {
    return useQuery<BookingCalendarType[]>(
        ["bookingsQuery", userID],
        async () => {
            const axe = await axiosService.get(`Bookings/${userID}/futureBookings`, {
            });

            return axe.data;
        },
        {
            enabled: !!userID,
            cacheTime: 0
        }
    );
};

export default getMyBookings;
