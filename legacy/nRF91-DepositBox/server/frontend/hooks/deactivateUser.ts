import { useMutation } from "react-query";
import { queryClient } from "../pages/_app";
import axiosService from "../services/axiosService";
import { UserType } from "../typings/userType";

const deactivateUser = () => {
    return useMutation({
        mutationFn: (data: UserType) => {
          return axiosService.put(`User/${data.id}`, {
            name: data.name,
            email: data.email,
            phone: data.phone,
            role: data.roles,
            isActive: data.isActive, 
          }) 
        },
        onSettled: () => {
          queryClient.invalidateQueries('userQuery');
      }})
    }

export default deactivateUser; 