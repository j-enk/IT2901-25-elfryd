import { Box, Typography } from "@mui/material"
import UnlockButton from "../../Buttons/UnlockButton"
import { useSelector } from "react-redux";
import { Dispatch, useEffect, useState } from "react";
import { selectSelectedBoat } from "../../../features/boat/boatSelector";
import { useAppDispatch } from "../../../features/hooks";
import { selectLockStatus } from "../../../features/lockbox/lockboxSelector";
import { fetchLockBoxes, setSelectedLockBox } from "../../../features/lockbox/lockboxSlice";
import adminOpenLockBoxMutation from "../../../hooks/lockbox/adminOpenLockBoxMutation";
import AdminDescription from "../adminDescription";

const actionDispatch = (dispatch: Dispatch<any>) => ({
    fetchLockBoxes: () => dispatch(fetchLockBoxes()),
    setLockBox: (query: number) => dispatch(setSelectedLockBox(query)),
  });


const AdminUnlockComponent = () => {
    const lockBoxStatus = useSelector(selectLockStatus);
    const selectedBoat = useSelector(selectSelectedBoat);
    const [open, setOpen] = useState(true);

    const handleShow = () => {
      setOpen((open) => !open);
    };
  
    const {
      mutate: openLockBox,
      isLoading: openLockBoxLoading,
      isSuccess: openLockBoxSuccess,
    } = adminOpenLockBoxMutation();
    
    const {
      fetchLockBoxes,
      setLockBox,
    } = actionDispatch(useAppDispatch());
    
    useEffect(() => {
      if (lockBoxStatus === "succeeded") {
        if (selectedBoat) {
          setLockBox(selectedBoat.lockBoxID);
        }
      }
    }, [lockBoxStatus]);
    
    useEffect(() => {
      if (lockBoxStatus === "idle") {
        fetchLockBoxes();
      }
    }, []);


    return(
    <Box display="flex" alignItems="center" flexWrap="wrap" flexDirection="column">
        <AdminDescription description="På denne siden kan du åpne nøkkelskapet ved behov, 
            Trykk på knappen under for å åpne nøkkelskapet:"/>
        <UnlockButton
          handleShow={handleShow}
          openLockBox={openLockBox}
          openLockBoxLoading={openLockBoxLoading}
          openLockBoxSuccess={openLockBoxSuccess}
        />
    </Box>
    );
}

export default AdminUnlockComponent;