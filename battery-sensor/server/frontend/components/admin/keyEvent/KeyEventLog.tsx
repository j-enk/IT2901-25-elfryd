import {
  Box,
  List,
  ListItem,
  ListItemIcon,
  ListItemText,
  Pagination,
} from "@mui/material";
import useKeyEvents from "../../../hooks/keyEvents/useKeyEvents";
import Loading from "../../loading/Loading";
import VpnKeyIcon from "@mui/icons-material/VpnKey";
import VpnKeyOffIcon from "@mui/icons-material/VpnKeyOff";
import { useEffect, useState } from "react";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";
import CustomSnackBar from "../../snackbar/CustomSnackbar";

type KeyEventLogProps = {
  lockBoxID: number;
  pageSize?: number;
};

const ConvertTimeStampToDateString = (timeStamp: string) => {
  const date = new Date(timeStamp);
  return date.toLocaleString("nb-NO");
};

const KeyEventActionToString = (action: number) => {
  return action == 0 ? "hentet" : "levert";
};

const KeyEventLog = (props: KeyEventLogProps) => {
  const { lockBoxID, pageSize } = props;
  const [pageIndex, setPageIndex] = useState(1);
  const { data, isLoading, isError } = useKeyEvents(
    lockBoxID,
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
      openSnackbar("Kunne ikke hente logg", "error");
    }
  }, [isError]);

  if (isLoading || !data) {
    return (
      <Box
        display="flex"
        justifyContent="center"
        alignItems="center"
        width="100%"
        marginTop="24px"
      >
        <Loading />
        <CustomSnackBar
          severity={snackbarSeverity}
          message={snackbarMessage}
          open={snackbarOpen}
          handleClose={closeSnackbar}
        />
      </Box>
    );
  }

  return (
    <>
      <List>
        {data &&
          data.items.map((keyEvent) => (
            <ListItem
              key={keyEvent.timeStamp}
              sx={{
                border: "1px solid black",
                borderRadius: "5px",
                marginY: "4px",
              }}
            >
              <ListItemIcon>
                {keyEvent.action === 1 ? (
                  <VpnKeyIcon color="success" />
                ) : (
                  <VpnKeyOffIcon color="error" />
                )}
              </ListItemIcon>
              <Box display="flex" flexDirection="column">
                <ListItemText sx={{ margin: "0px" }}>
                  {ConvertTimeStampToDateString(keyEvent.timeStamp)}
                </ListItemText>
                <ListItemText sx={{ margin: "0px" }}>
                  {keyEvent.userName} har{" "}
                  {KeyEventActionToString(keyEvent.action)} nøkkel
                </ListItemText>
              </Box>
            </ListItem>
          ))}
      </List>
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
    </>
  );
};

export default KeyEventLog;
