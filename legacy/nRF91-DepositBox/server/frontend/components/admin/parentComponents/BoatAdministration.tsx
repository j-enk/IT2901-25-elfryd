import { Box, Tab, Tabs, Typography } from "@mui/material";
import BoatListContainer from "../boats/BoatListContainer";
import NewBoatForm from "../boats/forms/NewBoatForm";
import { useState, useEffect } from "react";
import AdminDescription from "../adminDescription";

const BoatAdministration = () => {
  const [currentTab, setCurrentTab] = useState(0);

  const handleTabChange = (event: React.SyntheticEvent, newValue: number) => {
    setCurrentTab(newValue);
  };

  return (
    <Box width="100%" paddingBottom="24px">
      <Typography
        variant="h1"
        textAlign="center"
        fontSize="48px"
        marginY="18px"
      >
        Administrer båter
      </Typography>
      <AdminDescription description="Her er siden for å administrere båter. Velg mellom å se en liste over båtene eller legge til en ny båt i systemet" />
      <Tabs
        value={currentTab}
        onChange={handleTabChange}
        centered
        style={{ marginBottom: "24px" }}
      >
        <Tab label="Båter" />
        <Tab label="Legg til ny båt" />
      </Tabs>
      {currentTab === 0 && <BoatListContainer />}
      {currentTab === 1 && <NewBoatForm />}
    </Box>
  );
};

export default BoatAdministration;
