import { useMutation, useQuery } from "react-query";
import axiosService from "../../services/axiosService";
import { KeyReportType } from "../../typings/keyReportType";
import { queryClient } from "../../pages/_app";

export const generateKeyReport = () => {
  return useMutation({
    mutationFn: (id: string) => {
      return axiosService.post(`LockBoxes/${id}/generateKeyReport`);
    },
    onSettled: () => {
      queryClient.invalidateQueries("latestKeyReportQuery");
    },
  });
};

export const latestKeyReport = (id: string) => {
  return useQuery<KeyReportType>(["latestKeyReportQuery", id], async () => {
    const response = await axiosService.get(`Lockboxes/${id}/latestKeyReport`);
    return response.data;
  });
};
