import { Box, Pagination } from "@mui/material";
import { useEffect, useState } from "react";
import useUsers from "../../../hooks/users/useUsers";
import Loading from "../../loading/Loading";
import { UserListItem } from "./UserListItem";
import useCustomSnackbar from "../../snackbar/useCustomSnackbar";
import CustomSnackBar from "../../snackbar/CustomSnackbar";

type UserListProps = {
  search: string;
  pageSize?: number;
};

const UserList = (props: UserListProps) => {
  const { search, pageSize } = props;
  const [pageIndex, setPageIndex] = useState(1);
  const { data, isLoading, isError } = useUsers(search, pageIndex, pageSize);
  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke hente brukere", "error");
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
        Ingen brukere samsvarte med søket
      </Box>
    );

  return (
    <Box width="100%">
      <Box display="flex" gap="6px" flexDirection="column">
        {data.items.map((user) => (
          <UserListItem key={user.id} user={user}></UserListItem>
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

export default UserList;
