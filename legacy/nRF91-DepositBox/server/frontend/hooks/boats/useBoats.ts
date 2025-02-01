import { useQuery } from "react-query";
import axiosService from "../../services/axiosService";
import { BoatType } from "../../typings/boatType";
import { PaginationType } from "../../typings/paginationType";

const useBoats = (search: string, pageIndex?: number, pageSize?: number) => {
  return useQuery<PaginationType<BoatType>>(
    ["boatQuery", search, pageIndex, pageSize],
    async () => {
      const response = await axiosService.get("Boats/", {
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

export default useBoats;
