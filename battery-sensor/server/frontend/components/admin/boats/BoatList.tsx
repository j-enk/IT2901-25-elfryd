import { Box, Pagination } from "@mui/material";
import BoatItem from "./BoatItem";
import { useEffect, useState } from "react";
import useBoats from "../../../hooks/boats/useBoats";
import Loading from "../../loading/Loading";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";
import CustomSnackBar from "../../snackbar/CustomSnackbar";

type BoatListProps = {
  search: string;
  pageSize: number;
};

const BoatList = (props: BoatListProps) => {
  const { search, pageSize } = props;
  const [pageIndex, setPageIndex] = useState(1);
  const {
    data: boats,
    isLoading,
    isError,
  } = useBoats(search, pageIndex, pageSize);

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke hente båter", "error");
    }
  }, [isError]);

  if (isLoading || !boats)
    return (
      <Box width="100%" display="flex" justifyContent="center" marginTop="36px">
        <Loading />
        <CustomSnackBar
          severity={snackbarSeverity}
          message={snackbarMessage}
          open={snackbarOpen}
          handleClose={closeSnackbar}
        />
      </Box>
    );

  if (!boats.items.length)
    return (
      <Box width="100%" textAlign="center">
        Ingen båter samsvarte med søket
      </Box>
    );

  const boatItemElements = boats.items.map((boat) => {
    return <BoatItem key={boat.boatID} boat={boat} />;
  });

  return (
    <Box width="100%">
      <Box display="flex" gap="6px" flexDirection="column">
        {boatItemElements}
      </Box>
      <Box
        display="flex"
        alignItems="center"
        justifyContent="center"
        marginTop="16px"
      >
        <Pagination
          count={boats.totalPages}
          hideNextButton={!boats.hasNextPage}
          hidePrevButton={!boats.hasPreviousPage}
          page={boats.pageIndex}
          shape="rounded"
          onChange={(_, value) => setPageIndex(value)}
        ></Pagination>
      </Box>
      <CustomSnackBar
        severity={snackbarSeverity}
        message={snackbarMessage}
        open={snackbarOpen}
        handleClose={closeSnackbar}
      />
    </Box>
  );
};

export default BoatList;
