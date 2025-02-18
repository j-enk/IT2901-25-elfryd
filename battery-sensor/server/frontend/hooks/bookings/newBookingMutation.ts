import { Query, useMutation } from "react-query";
import { useSelector } from "react-redux";
import { selectSelectedBoat } from "../../features/boat/boatSelector";
import { selectActiveUser } from "../../features/user/userSelector";
import { queryClient } from "../../pages/_app";
import axiosService from "../../services/axiosService";
import { NewBookingFormType } from "../../typings/formTypes";

const useNewBookingMutation = () => {
  const boat = useSelector(selectSelectedBoat);
  const user = useSelector(selectActiveUser);

  return useMutation({
    mutationKey: "newBookingMutation",
    mutationFn: (data: NewBookingFormType) => {
      return axiosService.post("/Bookings", {
        startTime: data.from,
        endTime: data.to,
        comment: data.comment,
        isOfficial: data.isOfficial,
        status: "P",
        userID: user.id,
        boatID: boat.boatID === 0 ? 1 : boat.boatID,
      });
    },
    onSettled: () => {
      queryClient.invalidateQueries("bookingSpanQuery");
      queryClient.invalidateQueries("activeBookingQuery");
      queryClient.invalidateQueries("bookingsQuery");
    },
  });
};

export default useNewBookingMutation;
