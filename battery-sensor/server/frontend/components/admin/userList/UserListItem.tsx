import { Box, Button, Modal, Typography } from "@mui/material";
import { useEffect, useState } from "react";
import theme from "../../../theme";
import { UserType } from "../../../typings/userType";
import DeactivateButton from "./DeactivateButton";
import DeleteUserModal from "./DeleteUserModal";
import EditUserForm from "./forms/EditUserForm";

export interface UserListItemProps {
  user: UserType;
}

export const UserListItem = (props: UserListItemProps) => {
  const { user } = props;
  const [modalOpen, setModalOpen] = useState(false);
  const [edit, setEdit] = useState(false);
  const isActive = user.isActive;

  const [width, setWidth] = useState(window.innerWidth);

  useEffect(() => {
    window.addEventListener("resize", () => setWidth(window.innerWidth));
  }, []);

  const handleEdit = () => setEdit(!edit);
  const handleClose = () => {
    setModalOpen(false);
    setEdit(false);
  };

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
    <>
      <Box
        display="flex"
        border="1px solid black"
        borderRadius="4px"
        padding="8px"
        justifyContent="space-between"
        alignItems="center"
        sx={{ backgroundColor: isActive ? "white" : "#22222222" }}
      >
        <Typography variant="h6" component="h2">
          {user.name} {isActive ? "" : " (Deaktivert)"}
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
          {width > 700 && <DeleteUserModal id={user.id} trashcan={true} />}
        </Box>
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
            Detaljer for {user.name}
          </Typography>
          <Box
            display="flex"
            marginTop="16px"
            gap="32px"
            width="100%"
            justifyContent="space-around"
            flexWrap="wrap"
          >
            <Box display="flex" flexDirection="column" gap="12px">
              <EditUserForm
                user={user}
                edit={edit}
                setEdit={setEdit}
              ></EditUserForm>
              {!edit && (
                <>
                  <Button onClick={handleEdit} variant="outlined" color="info">
                    Rediger bruker
                  </Button>
                  <DeactivateButton user={user} />
                  <DeleteUserModal id={user.id} trashcan={false} />
                </>
              )}
            </Box>
          </Box>
        </Box>
      </Modal>
    </>
  );
};
