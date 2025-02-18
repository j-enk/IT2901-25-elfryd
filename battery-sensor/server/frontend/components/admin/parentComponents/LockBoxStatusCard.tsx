import { Box, Chip, IconButton, Typography } from "@mui/material";
import KeyIcon from "@mui/icons-material/Key";
import RefreshIcon from "@mui/icons-material/Refresh";

type LockBoxStatusCardProps = {
  title: string;
  subTitle: string;
  isActive: boolean;
  chipLabel: string;
  refreshFunction: () => void;
};

const LockBoxStatusCard = (props: LockBoxStatusCardProps) => {
  const { isActive, subTitle, title, chipLabel, refreshFunction } = props;
  return (
    <Box
      sx={{
        width: "100%",
        mx: "auto",
        border: "1px solid lightgrey",
        p: 2,
        borderRadius: 1,
      }}
    >
      <Box
        display="flex"
        flexDirection="row"
        width="100%"
        justifyContent="space-around"
      >
        <Box display="flex" justifyContent="center" alignItems="center">
          <KeyIcon sx={{ fontSize: 40, color: "gray" }} />
        </Box>
        <Box display="flex" flexDirection="column" paddingX={1}>
          <Typography fontSize="18px">{title}</Typography>
          <Typography fontSize="14px">{subTitle}</Typography>
        </Box>
        <Box display="flex" justifyContent="center" alignItems="center">
          <Chip
            sx={{ fontSize: "16px", paddingX: 2, paddingY: 1 }}
            color={isActive ? "success" : "error"}
            label={chipLabel}
          />
        </Box>
        <Box display="flex" justifyContent="center" alignItems="center">
          <IconButton onClick={() => refreshFunction()}>
            <RefreshIcon />
          </IconButton>
        </Box>
      </Box>
    </Box>
  );
};

export default LockBoxStatusCard;
