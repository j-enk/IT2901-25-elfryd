import { Box, CircularProgress } from "@mui/material";

export const Loading = () => {
    return (
      <Box sx={{ display: "flex" }}>
        <CircularProgress />
      </Box>
    );
  };

export default Loading; 