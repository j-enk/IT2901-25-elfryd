import { useEffect, useState } from "react";
import getMyBookings from "../../hooks/getMyBookings";
import { useSelector } from "react-redux";
import { Alert, Box, Card, Snackbar, Table, TableBody, TableCell, TableHead, TableRow, Typography } from "@mui/material";
import { selectActiveUser } from "../../features/user/userSelector";
import Loading from "../loading/Loading";
import { BookingCard } from "./BookingCard"
import ReportProblemIcon from '@mui/icons-material/ReportProblem';;
import useRenderDesktopView from "../../hooks/shouldRenderDesktopView";

interface MyBookingsComponentProps {
    handleOpenShow: () => void;
}

const MyBookingsComponent = (props: MyBookingsComponentProps) => {
    const { handleOpenShow } = props;

    const activeUser = useSelector(selectActiveUser);
    const [open, setOpen] = useState(false);

    const { data, isLoading, isError } = getMyBookings(activeUser.id);

    const shouldRenderDesktop = useRenderDesktopView();

    useEffect(() => {
        if (isError) {
            setOpen(true);
        }
    }, [isError]);

    const handleClose = () => {
        setOpen(false);
    };

    if (isLoading || !data)
        return (
            <Box width="100%" display="flex" justifyContent="center" marginTop="36px">
                <Loading />
            </Box>
        );

    if (data.length === 0) return (
        <Box marginX="auto" paddingTop="15%" width="40%" >
            <Card sx={{ padding: 7, display: "flex", flexDirection: "row", gap: "10px" }} style={{ backgroundColor: "#FAFAFA" }}>
                <Typography variant="h5">
                    Du har ingen kommende bookinger
                </Typography>
                <ReportProblemIcon fontSize="large" />
            </Card>
        </Box>
    )

    return (
        <Box marginX="auto" padding={5} width={shouldRenderDesktop ? "40%" : "90%"}>
            <Typography variant="h4">Dine kommende bookinger:</Typography>
            {data.map((booking, i) => (
                <BookingCard
                    key={booking.bookingID}
                    index={i}
                    startTime={booking.startTime}
                    boatName={booking.boatName}
                    endTime={booking.endTime}
                    handleOpenShow={handleOpenShow}
                />
            ))}
            <Snackbar open={open} autoHideDuration={6000} onClose={handleClose}>
                <Alert onClose={handleClose} severity="error" sx={{ width: "100%" }}>
                    Noe gikk galt under hentingen av bookingperiodene
                </Alert>
            </Snackbar>
        </Box>
    )
};

export default MyBookingsComponent;