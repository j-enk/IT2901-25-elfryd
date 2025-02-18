import { Box, Tab, Tabs, Typography } from "@mui/material";
import { useEffect, useState } from "react";
import UserListComponent from "../userList/UserListComponent";
import NewUserForm from "../userList/forms/NewUserForm";
import BookingPeriodComponent from "../bookingPeriod/BookingPeriodComponent";
import NewBookingPeriodForm from "../bookingPeriod/forms/NewBookingPeriodForm";
import AdminDescription from "../adminDescription";

const BookingPeriodAdministration: React.FC = () => {
  const [currentTab, setCurrentTab] = useState(0);
  const [shouldBeFullWidth, setShouldBeFullWidth] = useState<boolean>(false);

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setCurrentTab(newValue);
  };

  const handleResize = () => {
    setShouldBeFullWidth(window.innerWidth < 900);
  };

  useEffect(() => {
    window.addEventListener("resize", handleResize);
    return () => {
      window.removeEventListener("resize", handleResize);
    };
  }, []);

  return (
    <Box width="100%" paddingBottom="24px">
      <Typography
        variant="h1"
        textAlign="center"
        fontSize="48px"
        marginY="18px"
      >
        Administrer bookingperioder
      </Typography>
      <AdminDescription description="En bookingperiode er en lengre periode med tid der det er lov å booke båten. Hvis det ikke er noen aktive bookingperioder vil brukere ikke kunne booke båten. Du kan for eksempel legge til en bookingperiode til sommersesongen og åpne den i starten av juni og stenge den i slutten av august."/>
      <Tabs
        value={currentTab}
        onChange={handleTabChange}
        centered
        style={{ marginBottom: "24px" }}
        variant={shouldBeFullWidth ? "fullWidth" : "standard"}
      >
        <Tab label="Bookingperioder" />
        <Tab label="Legg til ny" />
      </Tabs>
      {currentTab === 0 && <BookingPeriodComponent />}
      {currentTab === 1 && <NewBookingPeriodForm />}
    </Box>
  );
};

export default BookingPeriodAdministration;
