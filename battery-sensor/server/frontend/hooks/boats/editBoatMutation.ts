import { useMutation } from "react-query";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";
import { EditBoatFormType } from "../../typings/formTypes";
import { BoatType } from "../../typings/boatType";

const editBoatMutation = () => {
  return useMutation({
    mutationFn: (data: EditBoatFormType) => {
      const editObject: BoatType = {
        name: data.Name,
        boatID: data.BoatID,
        chargingTime: data.ChargingTime,
        advanceBookingLimit: data.AdvanceBookingLimit,
        maxBookingLimit: data.MaxBookingLimit,
        lockBoxID: data.LockBoxID,
      };
      return axiosService.put<EditBoatFormType>(
        `Boats/${data.BoatID}`,
        editObject
      );
    },
    onSettled: () => {
      queryClient.invalidateQueries("boatQuery");
    },
  });
};

export default editBoatMutation;
