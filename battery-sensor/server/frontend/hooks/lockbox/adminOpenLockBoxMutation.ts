import { useMutation } from "react-query";
import axiosService from "../../services/axiosService";

const adminOpenLockBoxMutation = () => {
  return useMutation({
    mutationKey: "openLockBoxMutation",
    mutationFn: (lockBoxID: number) => {
      return axiosService.post(`LockBoxes/${lockBoxID}/adminOpen/`);
    },
  });
};

export default adminOpenLockBoxMutation;
