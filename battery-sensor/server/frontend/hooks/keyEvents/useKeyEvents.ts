import { useQuery } from "react-query";
import axiosService from "../../services/axiosService";
import { KeyEventsType } from "../../typings/keyEvent";
import { PaginationType } from "../../typings/paginationType";

const useKeyEvents = (
  lockBoxID: number,
  pageIndex?: number,
  pageSize?: number
) => {
  return useQuery<PaginationType<KeyEventsType>>(
    ["keyEventQuery", lockBoxID, pageIndex, pageSize],
    async () => {
      const URL = `LockBoxes/${lockBoxID}/allKeyStatus`;

      const response = await axiosService.get(URL, {
        params: {
          pageIndex: pageIndex,
          pageSize: pageSize,
        },
      });
      return response.data;
    },
    { enabled: lockBoxID !== null && lockBoxID > 0 }
  );
};

export default useKeyEvents;
