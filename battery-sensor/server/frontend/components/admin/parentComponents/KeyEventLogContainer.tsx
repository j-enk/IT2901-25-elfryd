import {
  Box,
  FormControl,
  InputLabel,
  MenuItem,
  Select,
  Typography,
} from "@mui/material";
import { Dispatch, useEffect, useState } from "react";
import { useSelector } from "react-redux";
import { selectSelectedBoat } from "../../../features/boat/boatSelector";
import { useAppDispatch } from "../../../features/hooks";
import KeyEventLog from "../keyEvent/KeyEventLog";
import {
  selectLockStatus,
  selectLockboxes,
} from "../../../features/lockbox/lockboxSelector";
import { fetchLockBoxes } from "../../../features/lockbox/lockboxSlice";
import AdminDescription from "../adminDescription";

const actionDispatch = (dispatch: Dispatch<any>) => ({
  fetchLockBoxes: () => dispatch(fetchLockBoxes()),
});

const KeyEventLogContainer = () => {
  const selectedBoat = useSelector(selectSelectedBoat);
  const lockboxStatus = useSelector(selectLockStatus);
  const lockboxes = useSelector(selectLockboxes);
  const [selectedLockBox, setSelectedLockBox] = useState(
    selectedBoat.lockBoxID ? selectedBoat.lockBoxID : 1
  );
  const [pageSize, setPageSize] = useState(25);

  const { fetchLockBoxes } = actionDispatch(useAppDispatch());

  useEffect(() => {
    if (lockboxStatus === "idle") {
      fetchLockBoxes();
    }
  }, []);

  return (
    <Box width="75%" marginX="auto">
      <Typography
        variant="h1"
        textAlign="center"
        fontSize="48px"
        marginY="18px"
      >
        Nøkkelhistorikk
      </Typography>
      <AdminDescription description="Dette er en oversikt over alle som har levert og fjernet nøkkelen fra nøkkelskapet. Om nøkkelen er borte kan man se her hvem som hadde den sist"/>
      <Box
        display="flex"
        flexDirection="row"
        gap="12px"
        justifyContent="space-between"
        sx={{ width: { xs: "100%", md: "50%", lg: "50%" } }}
      >
        <FormControl fullWidth>
          <InputLabel id="selectLockBoxLabel">Velg nøkkelskap</InputLabel>
          <Select
            labelId="selectLockBoxLabel"
            id="selectLockBox"
            value={selectedLockBox}
            label="Velg Nøkkelskap"
            onChange={(event, _) =>
              setSelectedLockBox(event.target.value as number)
            }
          >
            {lockboxes.map((lockbox) => (
              <MenuItem key={lockbox.lockBoxID} value={lockbox.lockBoxID}>
                Nøkkelskap {lockbox.lockBoxID}
              </MenuItem>
            ))}
          </Select>
        </FormControl>

        <FormControl fullWidth>
          <InputLabel id="selectPageSizeLabel">
            Antall elementer per side
          </InputLabel>
          <Select
            labelId="selectPageSizeLabel"
            id="selectPageSize"
            value={pageSize}
            label="Antall elementer per side"
            onChange={(event, _) => setPageSize(event.target.value as number)}
          >
            <MenuItem value={10}>10</MenuItem>
            <MenuItem value={25}>25</MenuItem>
            <MenuItem value={30}>50</MenuItem>
          </Select>
        </FormControl>
      </Box>
      <KeyEventLog lockBoxID={selectedLockBox} pageSize={pageSize} />
    </Box>
  );
};

export default KeyEventLogContainer;
