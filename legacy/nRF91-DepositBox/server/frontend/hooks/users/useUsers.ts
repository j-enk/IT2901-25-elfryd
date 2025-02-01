import { useQuery } from "react-query";
import axiosService from "../../services/axiosService";
import { PaginationType } from "../../typings/paginationType";
import { UserType } from "../../typings/userType";

export const useUsers = (
  search: string,
  pageIndex?: number,
  pageSize?: number
) => {
  return useQuery<PaginationType<UserType>>(
    ["userQuery", pageIndex, pageSize, search],
    async () => {
      const response = await axiosService.get("User", {
        params: {
          search: search,
          pageIndex: pageIndex,
          pageSize: pageSize,
        },
      });

      return response.data;
    }
  );
};

export default useUsers;
