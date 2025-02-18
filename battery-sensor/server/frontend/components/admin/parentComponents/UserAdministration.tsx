import { Box, Tab, Tabs, Typography } from "@mui/material";
import { useEffect, useState } from "react";
import UserListComponent from "../userList/UserListComponent";
import NewUserForm from "../userList/forms/NewUserForm";
import AdminDescription from "../adminDescription";

const UserAdministration: React.FC = () => {
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
        Administrer brukere
      </Typography>
      <AdminDescription description="Her ser du en liste over brukere som er lagt inn i systemet og du kan også legge inn nye brukere."/>
      <Tabs
        value={currentTab}
        onChange={handleTabChange}
        centered
        style={{ marginBottom: "24px" }}
        variant={shouldBeFullWidth ? "fullWidth" : "standard"}
      >
        <Tab label="Brukere" />
        <Tab label="Legg til ny bruker" />
      </Tabs>
      {currentTab === 0 && <UserListComponent />}
      {currentTab === 1 && <NewUserForm />}
    </Box>
  );
};

export default UserAdministration;
