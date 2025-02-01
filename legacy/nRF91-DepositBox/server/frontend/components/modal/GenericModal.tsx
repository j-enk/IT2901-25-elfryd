import {
  Dialog,
  Box,
  DialogTitle,
  IconButton,
  DialogContent,
} from "@mui/material";

import CloseIcon from "@mui/icons-material/Close";
type GenericModalProps = {
  open: boolean;
  handleShow: () => void;
  title: string;
  children: React.ReactNode;
};

const GenericModal = (props: GenericModalProps) => {
  const { open, handleShow, title, children } = props;

  return (
    <Dialog
      onClose={handleShow}
      open={open}
      sx={{
        display: "flex",
        flexDirection: "column",
        alignItems: "center",
        justifyContent: "center",
      }}
    >
      <Box display="flex" justifyContent="space-between" alignItems="center">
        <DialogTitle sx={{ paddingBottom: "0px" }}>{title}</DialogTitle>
        <IconButton
          aria-label="close"
          onClick={handleShow}
          sx={{
            color: (theme) => theme.palette.grey[500],
          }}
        >
          <CloseIcon />
        </IconButton>
      </Box>
      <DialogContent
        sx={{
          display: "flex",
          flexDirection: "column",
          alignItems: "center",
          width: ["80vw", "70vw", "45vw", "40vw", "30vw", "20vw"],
          paddingTop: "0px",
        }}
      >
        {children}
      </DialogContent>
    </Dialog>
  );
};

export default GenericModal;
