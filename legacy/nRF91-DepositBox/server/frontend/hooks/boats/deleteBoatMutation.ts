import { useMutation } from "react-query";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";

const deleteBoatMutation = () => {
  return useMutation({
    mutationFn: (id: number) => {
      return axiosService.delete(`Boats/${id}`);
    },
    onSettled: () => {
      queryClient.invalidateQueries("boatQuery");
    },
  });
};

export default deleteBoatMutation;
