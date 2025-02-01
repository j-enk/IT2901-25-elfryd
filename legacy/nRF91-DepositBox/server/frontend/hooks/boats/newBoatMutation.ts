import { useMutation } from "react-query";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";
import { NewBoatFormType } from "../../typings/formTypes";

const newBoatMutation = () => {
  return useMutation({
    mutationFn: (data: NewBoatFormType) => {
      return axiosService.post("/Boats", data);
    },
    onSettled: () => {
      queryClient.invalidateQueries("boatQuery");
    },
  });
};

export default newBoatMutation;
