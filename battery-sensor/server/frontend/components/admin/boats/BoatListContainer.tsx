import {
  Box,
  FormControl,
  InputLabel,
  MenuItem,
  Select,
  TextField,
} from "@mui/material";
import BoatList from "./BoatList";
import { useState } from "react";

const BoatListContainer = () => {
  const [pageSize, setPageSize] = useState(25);
  const [search, setSearch] = useState("");

  return (
    <Box marginX="auto" sx={{ width: { xs: "80%", md: "75%", lg: "50%" } }}>
      <Box
        display="flex"
        flexDirection="row"
        justifyContent="space-between"
        alignItems="flex-end"
        marginBottom="16px"
      >
        <Box width="20%">
          <FormControl fullWidth>
            <InputLabel id="selectPageSizeLabel">
              Antall båter per side
            </InputLabel>
            <Select
              labelId="selectPageSizeLabel"
              id="selectPageSize"
              value={pageSize}
              label="Antall båter per side"
              onChange={(event, _) => setPageSize(event.target.value as number)}
            >
              <MenuItem value={10}>10</MenuItem>
              <MenuItem value={25}>25</MenuItem>
              <MenuItem value={30}>50</MenuItem>
            </Select>
          </FormControl>
        </Box>

        <FormControl>
          <TextField
            placeholder="Båtsøk"
            onChange={(ev) => setSearch(ev.target.value)}
          />
        </FormControl>
      </Box>
      <BoatList search={search} pageSize={pageSize} />
    </Box>
  );
};

export default BoatListContainer;
