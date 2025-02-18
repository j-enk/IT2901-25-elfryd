import { useMutation } from "react-query";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";
import { EditUserFormType } from "../../typings/formTypes";

const editUser = () => {
  return useMutation({
    mutationFn: (data: EditUserFormType) => {
      return axiosService.put(`User/${data.id}`, {
        name: data.Name,
        email: data.Email,
        phone: data.Phone,
        role: data.Role,
        bookingIDs: data.bookingIDs,
        isActive: data.isActive,
      });
    },
    onSettled: () => {
      queryClient.invalidateQueries("userQuery");
    },
  });
};

export default editUser;
