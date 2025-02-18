import { Box, Pagination } from "@mui/material";
import { useEffect, useState } from "react";
import getBookingPeriods from "../../../hooks/getBookingPeriods";
import Loading from "../../loading/Loading";
import { BookingPeriodListItem } from "./BookingPeriodListItem";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";
import CustomSnackBar from "../../snackbar/CustomSnackbar";

export interface BookingPeriodListProps {
  search: string;
  pageSize?: number;
  selectedBoat: number;
}

const BookingPeriodList = (props: BookingPeriodListProps) => {
  const { search, pageSize, selectedBoat } = props;
  const [pageIndex, setPageIndex] = useState(1);
  const { data, isLoading, isError } = getBookingPeriods(
    search,
    selectedBoat,
    pageIndex,
    pageSize
  );
  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke hente åpningsperioder", "error");
    }
  }, [isError]);

  if (isLoading || !data)
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

  if (!data.items.length)
    return (
      <Box width="100%" textAlign="center">
        Ingen bookingperioder samsvarte med søket
      </Box>
    );

  return (
    <Box width="100%">
      <Box display="flex" gap="6px" flexDirection="column">
        {data.items.map((bookingPeriod, i) => (
          <BookingPeriodListItem key={i} bookingPeriod={bookingPeriod} />
        ))}
      </Box>
      <Box
        display="flex"
        alignItems="center"
        justifyContent="center"
        marginTop="16px"
      >
        <Pagination
          count={data.totalPages}
          hideNextButton={!data.hasNextPage}
          hidePrevButton={!data.hasPreviousPage}
          page={data.pageIndex}
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

export default BookingPeriodList;
