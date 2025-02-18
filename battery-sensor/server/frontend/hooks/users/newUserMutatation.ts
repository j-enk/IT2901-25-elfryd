import { useMutation } from "react-query";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";
import { NewUserFormType } from "../../typings/formTypes";
import { PaginationType } from "../../typings/paginationType";
import { UserType } from "../../typings/userType";

const useNewUserMutation = () => {
  return useMutation({
    mutationFn: (data: NewUserFormType) => {
      return axiosService.post("/User", {
        name: data.Name,
        email: data.Email,
        phone: data.Phone,
        role: data.Role,
        isActive: data.isActive, 
      });
    },
    onMutate: async (data: NewUserFormType) => {
      await queryClient.cancelQueries("userQuery");
      const previousUsers =
        queryClient.getQueryData<PaginationType<UserType>>("userQuery");
      if (previousUsers) {
        queryClient.setQueryData<PaginationType<UserType>>("userQuery", {
          ...previousUsers,
          items: [
            ...previousUsers.items,
            {
              id: "99999",
              name: data.Name,
              email: data.Email,
              phone: data.Phone,
              roles: [data.Role],
              isActive: data.isActive,
            },
          ],
        });
      }
      return { previousUsers };
    },
    onSettled: () => {
      queryClient.invalidateQueries("userQuery");
    },
  });
};

export default useNewUserMutation;
