import { Box, Button, Card, CardContent, Table, TableBody, TableCell, TableRow, Typography } from "@mui/material"
import useUserHasActiveBooking from "../../hooks/bookings/useUserHasActiveBooking";
import { DirectionsBoat } from "@mui/icons-material";
import DirectionsBoatIcon from '@mui/icons-material/DirectionsBoat';
import { useState } from "react";
import useRenderDesktopView from "../../hooks/shouldRenderDesktopView";

export interface BookingCardProps {
    index: number,
    boatName: string,
    startTime: string,
    endTime: string,
    handleOpenShow: () => void;
}

export const BookingCard = (props: BookingCardProps) => {
    const { index, boatName, startTime, endTime, handleOpenShow } = props;

    const { data: userHasActiveBooking } = useUserHasActiveBooking();

    const [edit, setEdit] = useState(false);

    const shouldRenderDesktop = useRenderDesktopView();

    const monthNames = [
        "januar", "februar", "mars",
        "april", "mai", "juni", "juli",
        "august", "september", "oktober",
        "november", "desember"
    ];

    function formatDate(date: string) {
        const day = date.substring(8, 10)
        const month = date.substring(5, 7)
        const year = date.substring(0, 4)

        return (
            day + '/' + month + '/' + year
        )
    }

    function formatDateTitle(date: string): string {
        const day = date.substring(8, 10)
        const month = date.substring(5, 7)

        return (
            parseInt(day) + '. ' + monthNames[parseInt(month) - 1]
        )
    }

    function formatTime(startDate: string, endDate: string) {
        const openingTime = startDate.substring(11, 16)
        const closingTime = endDate.substring(11, 16)

        return (
            openingTime + ' - ' + closingTime
        )
    }

    function handleEdit() {
        setEdit(!edit);
    }

    return (
        <Card sx={{ marginTop: 2, display: "flex", borderRadius: "16px" }} style={{ backgroundColor: "#FAFAFA" }}>
            <CardContent sx={{ width: "100%" }}>
                <Box sx={{ display: "flex", flexDirection: "column" }}>
                    <Box sx={{ display: "flex", flexDirection: "row" }}>

                        <Typography variant="h4" marginLeft={1} style={{ fontWeight: "bold" }}>
                            {formatDateTitle(startTime)}
                        </Typography>
                        <DirectionsBoatIcon style={{ marginLeft: "auto", width: 40, height: 40 }} />
                    </Box>
                    <Typography marginLeft={1} style={{ fontStyle: "italic" }}>
                        {boatName}
                    </Typography>
                    <Typography marginLeft={1} marginTop={1} marginBottom={2} variant={shouldRenderDesktop ? "h5" : "h6"} style={{ fontWeight: shouldRenderDesktop ? "normal" : "bold" }}>
                        Klokkeslett: {formatTime(startTime, endTime)}
                    </Typography>


                    <Box sx={{ display: "flex", flexDirection: "row" }}>

                        {(userHasActiveBooking && index == 0) && (
                            <Button
                                onClick={handleOpenShow}
                                className="text-white mr-3 border-white bg-slate-700 ml-2"
                                endIcon={<DirectionsBoat />}
                            >
                                LÅS OPP
                            </Button>
                        )}
                    </Box>
                </Box>
            </CardContent>
        </Card>
    )
};