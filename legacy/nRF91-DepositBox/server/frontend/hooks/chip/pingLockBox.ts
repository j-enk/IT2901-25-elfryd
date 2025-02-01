import { useQuery } from "react-query"
import axiosService from "../../services/axiosService";
import { LockBoxType } from "../../typings/lockBoxType";

export const pingQuery = (
    id:string
) => {
    return useQuery<String>(["pingLockBoxQuery", id],
            async () => {
                const response = await axiosService.get(`LockBoxes/${id}/pingLockBox`)
                return response.data; 
            });

        };

export const getLockBox = (id:string) => {
    return useQuery<LockBoxType>(["getLockBoxQuery", id], 
    async () => {
        const response = await axiosService.get(`Lockboxes/${id}`)
        return response.data;
    });
}