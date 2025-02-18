import { useMutation } from "react-query";
import axiosService from "../../services/axiosService";

const openLockBoxMutation = () => {
  return useMutation({
    mutationKey: "openLockBoxMutation",
    mutationFn: (lockBoxID: number) => {
      return axiosService.post(`LockBoxes/${lockBoxID}/open/`);
    },
  });
};

export default openLockBoxMutation;
