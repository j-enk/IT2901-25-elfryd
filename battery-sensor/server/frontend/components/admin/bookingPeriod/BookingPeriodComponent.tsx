import {
  Box,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  TextField,
  SelectChangeEvent,
} from "@mui/material";
import { useState } from "react";
import BookingPeriodList from "./BookingPeriodList";
import { useSelector } from "react-redux";
import { selectSelectedBoat } from "../../../features/boat/boatSelector";
import useBoats from "../../../hooks/boats/useBoats";
import Loading from "../../loading/Loading";

const UserListComponent = () => {
  const [pageSize, setPageSize] = useState(25);
  const [search, setSearch] = useState("");
  const globalSelectedBoat = useSelector(selectSelectedBoat);
  const { data: boats, isLoading: isBoatsLoading } = useBoats("", 1, 999);

  const [selectedBoat, setSelectedBoat] = useState(
    globalSelectedBoat.boatID > 0 ? globalSelectedBoat.boatID : 0
  );

  function handleBoatChange(event: SelectChangeEvent<string | number>) {
    setSelectedBoat(Number(event.target.value));
  }
  if (!boats || isBoatsLoading)
    return (
      <Box width="100%" display="flex" justifyContent="center">
        <Loading />
      </Box>
    );

  return (
    <Box marginX="auto" sx={{ width: { xs: "80%", md: "75%", lg: "50%" } }}>
      <Box
        display="flex"
        flexDirection="row"
        justifyContent="space-between"
        alignItems="flex-end"
        marginBottom="16px"
        flexWrap="wrap"
      >
        <Box
          sx={{ width: { xs: "100%", md: "40%", lg: "40%" } }}
          display="flex"
          flexDirection="row"
          gap="10px"
        >
          <FormControl fullWidth>
            <InputLabel id="selectPageSizeLabel">Antall per side</InputLabel>
            <Select
              labelId="selectPageSizeLabel"
              id="selectPageSize"
              value={pageSize}
              label="Antall bookingperioder per side"
              onChange={(event, _) => setPageSize(event.target.value as number)}
            >
              <MenuItem value={10}>10</MenuItem>
              <MenuItem value={25}>25</MenuItem>
              <MenuItem value={50}>50</MenuItem>
            </Select>
          </FormControl>
          <FormControl fullWidth>
            <InputLabel id="selectPageSizeLabel">Båt</InputLabel>
            <Select
              labelId="label_boat_selector"
              id="boat_selector"
              label="Båt"
              value={selectedBoat}
              onChange={handleBoatChange}
            >
              {boats.items.map((boat) => (
                <MenuItem key={boat.boatID} value={boat.boatID}>
                  {boat.name}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Box>

        <FormControl>
          <TextField
            placeholder="Bookingperiode-søk"
            onChange={(ev) => setSearch(ev.target.value)}
          />
        </FormControl>
      </Box >

      <BookingPeriodList
        search={search}
        pageSize={pageSize}
        selectedBoat={selectedBoat}
      />
    </Box >
  );
};

export default UserListComponent;
