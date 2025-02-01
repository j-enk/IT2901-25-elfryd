import { Box, Button, Modal, Typography } from "@mui/material";
import { useState } from "react";
import theme from "../../../theme";
import { BoatType } from "../../../typings/boatType";
import EditBoatForm from "./forms/EditBoatForm";
import { DeleteBoatModal } from "./DeleteBoatModal";

type BoatDetailModalProps = {
  boatDetails: BoatType;
};

const BoatDetailModal = (props: BoatDetailModalProps) => {
  const { boatDetails } = props;
  const [modalOpen, setModalOpen] = useState(false);
  const [edit, setEdit] = useState(false);

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
      <Button
        variant="contained"
        color="info"
        sx={{ color: "black", backgroundColor: "white", width: "200%" }}
        onClick={() => setModalOpen(true)}
      >
        Se detaljer
      </Button>

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
            Detaljer for {boatDetails.name}
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
              <EditBoatForm
                boat={boatDetails}
                edit={edit}
                setEdit={setEdit}
              ></EditBoatForm>
              {!edit && (
                <>
                  <Button onClick={handleEdit} variant="outlined" color="info">
                    Rediger båt
                  </Button>
                  <DeleteBoatModal
                    boatID={boatDetails.boatID}
                    trashcan={false}
                  />
                </>
              )}
            </Box>
          </Box>
        </Box>
      </Modal>
    </>
  );
};

export default BoatDetailModal;
