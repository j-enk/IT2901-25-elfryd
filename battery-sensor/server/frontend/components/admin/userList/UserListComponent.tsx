import {
  Box,
  FormControl,
  InputLabel,
  Select,
  MenuItem,
  TextField,
} from "@mui/material";
import { useState } from "react";
import UserList from "./UserList";

const UserListComponent = () => {
  const [pageSize, setPageSize] = useState(25);
  const [search, setSearch] = useState("");

  return (
    <Box marginX="auto" sx={{ width: { xs: "80%", md: "75%", lg: "50%" }, }}>
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
              Antall brukere per side
            </InputLabel>
            <Select
              labelId="selectPageSizeLabel"
              id="selectPageSize"
              value={pageSize}
              label="Antall brukere per side"
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
            placeholder="Brukersøk"
            onChange={(ev) => setSearch(ev.target.value)}
          />
        </FormControl>
      </Box>

      <UserList search={search} pageSize={pageSize} />
    </Box>
  );
};

export default UserListComponent;
