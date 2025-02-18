import { useMutation } from "react-query";
import axiosService from "../services/axiosService";
import { NewBookingPeriodFormType } from "../typings/formTypes";
import { queryClient } from "../pages/_app";

const useNewBookingPeriodMutation = () => {
  return useMutation({
    mutationFn: (data: NewBookingPeriodFormType) => {
      return axiosService.post("/BookingPeriods", {
        name: data.Name,
        startDate: data.StartDate + "T00:00:00",
        endDate: data.EndDate + "T23:59:59",
        bookingOpens: data.BookingOpens,
        bookingCloses: data.BookingCloses,
        boatID: data.BoatID,
      });
    },
    //Refetches the booking periods in the calender after creating new booking period
    onSettled: () => {
      queryClient.invalidateQueries("BookingPeriodsQuery");
    },
  });
};

export default useNewBookingPeriodMutation;
