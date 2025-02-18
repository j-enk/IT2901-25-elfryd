import { useMutation } from "react-query";
import { queryClient } from "../pages/_app";
import axiosService from "../services/axiosService";

const deleteBookingPeriodMutation = () => {
  return useMutation({
    mutationFn: (arg: { name: string; boatID: number }) => {
      return axiosService.delete(
        `BookingPeriods/name=${arg.name}&boat=${arg.boatID}`
      );
    },
    onSettled: () => {
      queryClient.invalidateQueries("BookingPeriodsQuery");
    },
  });
};

export default deleteBookingPeriodMutation;
