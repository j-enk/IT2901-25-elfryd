import { useQuery } from "react-query";
import axiosService from "../services/axiosService";
import { BookingPeriodType } from "../typings/bookingPeriodType";
import { PaginationType } from "../typings/paginationType";

export const getBookingPeriods = (
  search: string,
  selectedBoat?: number,
  pageIndex?: number,
  pageSize?: number
) => {
  return useQuery<PaginationType<BookingPeriodType>>(
    ["BookingPeriodsQuery", search, pageIndex, pageSize, selectedBoat],
    async () => {
      const axe = await axiosService.get(
        `BookingPeriods/${selectedBoat ? selectedBoat + "/" : null}`,
        {
          params: {
            search: search,
            pageIndex: pageIndex,
            pageSize: pageSize,
          },
        }
      );

      return axe.data;
    },
    {
      enabled: selectedBoat != 0,
    }
  );
};

export default getBookingPeriods;
