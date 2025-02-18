import { Box, Typography, useMediaQuery } from "@mui/material";
import { BoatType } from "../../../typings/boatType";
import BoatDetailModal from "./BoatDetailModal";
import { DeleteBoatModal } from "./DeleteBoatModal";

type BoatItemProps = {
  boat: BoatType;
};

const BoatItem = (props: BoatItemProps) => {
  const { boat } = props;
  const shouldHideDelete = useMediaQuery("(max-width: 700px)");
  return (
    <Box
      display="flex"
      border="1px solid black"
      borderRadius="4px"
      padding="8px"
      justifyContent="space-between"
      alignItems="center"
    >
      <Typography variant="h6" component="h2">
        {boat.name}
      </Typography>
      <Box
        display="flex"
        sx={{
          width: "50%",
        }}
      >
        <BoatDetailModal boatDetails={boat} />
        {!shouldHideDelete && <DeleteBoatModal boatID={boat.boatID} trashcan />}
      </Box>
    </Box>
  );
};

export default BoatItem;
