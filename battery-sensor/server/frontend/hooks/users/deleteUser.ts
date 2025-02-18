import { useMutation } from "react-query";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";

const deleteUserMutation = () => {
  return useMutation({
    mutationFn: (id: string) => {
      return axiosService.delete(`User/${id}`);
    },
    onSettled: () => {
      queryClient.invalidateQueries("userQuery");
    },
  });
};

export default deleteUserMutation;
