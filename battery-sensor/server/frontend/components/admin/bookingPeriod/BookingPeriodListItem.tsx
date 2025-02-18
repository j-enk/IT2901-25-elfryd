import {
  Box,
  Button,
  Modal,
  Table,
  TableCell,
  TableContainer,
  TableRow,
  Typography,
} from "@mui/material";
import { useEffect, useState } from "react";
import theme from "../../../theme";
import { BookingPeriodType } from "../../../typings/bookingPeriodType";
import { DeleteBookingPeriodModal } from "./DeleteBookingPeriodModal";
import useBoats from "../../../hooks/boats/useBoats";

export interface BoookingPeriodListItemProps {
  bookingPeriod: BookingPeriodType;
}

export const BookingPeriodListItem = (props: BoookingPeriodListItemProps) => {
  const { bookingPeriod } = props;
  const [modalOpen, setModalOpen] = useState(false);
  const { data } = useBoats("", 1, 999);

  const handleClose = () => {
    setModalOpen(false);
  };

  function formatDate(date: string) {
    const day = date.substring(8, 10);
    const month = date.substring(5, 7);
    const year = date.substring(0, 4);

    return day + "/" + month + "/" + year;
  }

  const [width, setWidth] = useState(window.innerWidth);

  useEffect(() => {
    window.addEventListener("resize", () => setWidth(window.innerWidth));
  }, []);

  const modalStyle = {
    position: "absolute" as "absolute",
    top: "50%",
    left: "50%",
    transform: "translate(-50%, -50%)",
    [theme.breakpoints.down("sm")]: {
      width: "80%",
    },
    [theme.breakpoints.up("md")]: {
      width: "50%",
    },
    [theme.breakpoints.up("lg")]: {
      width: "30%",
    },
    [theme.breakpoints.up("xl")]: {
      width: "25%",
    },

    bgcolor: "background.paper",
    border: "2px solid #000",
    borderRadius: "4px",
    boxShadow: 24,
    p: 4,
  };

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
        {bookingPeriod.name}
      </Typography>
      <Box
        display="flex"
        sx={{
          width: "50%",
        }}
      >
        <Button
          variant="contained"
          color="info"
          sx={{ color: "black", backgroundColor: "white", width: "200%" }}
          onClick={() => setModalOpen(true)}
        >
          Se detaljer
        </Button>
        {width > 700 && (
          <DeleteBookingPeriodModal
            name={bookingPeriod.name}
            boatID={bookingPeriod.boatID}
            trashcan={true}
          />
        )}
      </Box>

      <Modal
        open={modalOpen}
        onClose={() => handleClose()}
        aria-labelledby="modal-modal-title"
        aria-describedby="modal-modal-description"
      >
        <Box sx={modalStyle}>
          <Typography
            id="modal-modal-title"
            variant="h6"
            component="h2"
            color="black"
            textAlign="center"
          >
            Detaljer for {bookingPeriod.name}
          </Typography>
          <Box
            display="flex"
            marginTop="16px"
            gap="32px"
            width="100%"
            justifyContent="center"
            flexWrap="wrap"
          >
            <Box display="flex" flexDirection="column" gap="12px">
              <Table>
                <TableRow>
                  <TableCell>Båt:</TableCell>
                  <TableCell align="right">
                    {
                      data?.items.find(
                        (boat) => boat.boatID === bookingPeriod.boatID
                      )!.name
                    }
                  </TableCell>
                </TableRow>
                <TableRow>
                  <TableCell>Startdato:</TableCell>
                  <TableCell align="right">
                    {formatDate(bookingPeriod.startDate)}
                  </TableCell>
                </TableRow>
                <TableRow>
                  <TableCell>Sluttdato:</TableCell>
                  <TableCell align="right">
                    {formatDate(bookingPeriod.endDate)}
                  </TableCell>
                </TableRow>
                <TableRow>
                  <TableCell>Åpningstid:</TableCell>
                  <TableCell align="right">
                    {bookingPeriod.bookingOpens}:00
                  </TableCell>
                </TableRow>
                <TableRow>
                  <TableCell>Stengetid:</TableCell>
                  <TableCell align="right">
                    {bookingPeriod.bookingCloses}:00
                  </TableCell>
                </TableRow>
              </Table>
              <DeleteBookingPeriodModal
                name={bookingPeriod.name}
                boatID={bookingPeriod.boatID}
                trashcan={false}
              />
            </Box>
          </Box>
        </Box>
      </Modal>
    </Box>
  );
};
