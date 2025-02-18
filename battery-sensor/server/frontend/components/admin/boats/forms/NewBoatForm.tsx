import {
  Box,
  Button,
  InputLabel,
  MenuItem,
  Select,
  TextField,
} from "@mui/material";
import { useEffect } from "react";
import { useForm } from "react-hook-form";
import { NewBoatFormType } from "../../../../typings/formTypes";
import CustomSnackBar from "../../../snackbar/CustomSnackbar";
import newBoatMutation from "../../../../hooks/boats/newBoatMutation";
import { useSelector } from "react-redux";
import { selectLockboxes } from "../../../../features/lockbox/lockboxSelector";
import useCustomSnackbar from "../../../snackbar/useCustomSnackbar";
import CustomToolTips from "../../CustomToolTips";

const NewBoatForm = () => {
  const { register, handleSubmit, getValues, setValue, reset } =
    useForm<NewBoatFormType>();
  const { mutate, isSuccess, isError } = newBoatMutation();

  const lockboxes = useSelector(selectLockboxes);

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  const onSubmit = (data: NewBoatFormType) => {
    mutate(data);
    reset();
  };

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Båten ble lagt til", "success");
    }
  }, [isSuccess]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke legge til ny båt", "error");
    }
  }, [isError]);

  return (
    <Box width="70%" marginX="auto">
      <form onSubmit={handleSubmit(onSubmit)}>
        <Box
          display="flex"
          flexDirection="column"
          gap="8px"
          marginX="auto"
          className="lg:w-8/12 xl:w-6/12"
        >
          <CustomToolTips title="Navn" description="Gi den nye båten et navn">
            <TextField
              label={"Navn"}
              fullWidth={true}
              required
              {...register("Name")}
            />
          </CustomToolTips>
          <CustomToolTips title="Ladetid" description="Prosentandelen du skriver inn her vil representere hvor mange prosent av bookingen sin tid ladetiden blir. Så om ladetid er 50% og noen booker båten en time, vil ladetiden være en halvtime">
            <TextField
              required
              type="number"
              fullWidth={true}
              label={"Ladetid (prosent per tid)"}
              {...register("ChargingTime")}
            ></TextField>
          </CustomToolTips>
          <CustomToolTips title="Forhåndsbooking" description="Hvor mange dager i forveien man kan booke båten">
          <TextField
            required
            type="number"
            fullWidth={true}
            label={"Forhåndsbooking (dager)"}
            {...register("AdvanceBookingLimit")}
          ></TextField>
          </CustomToolTips>
          <CustomToolTips title="Maks bookinglengde" description="Hvor mange timer en kan booke båten om gangen">
          <TextField
            required
            type="number"
            label={"Maks bookinglengde (timer)"}
            fullWidth={true}
            {...register("MaxBookingLimit")}
          ></TextField>
          </CustomToolTips>

          <InputLabel id="selectLockboxLabel">Nøkkelskap ID</InputLabel>
          <CustomToolTips title="Nøkkelskap ID" description="Om det er flere nøkkelskap kan du velge mellom dem her ved å bruke ID-nummeret deres">
          <Select
            fullWidth={true}
            required
            label={"Nøkkelskap ID"}
            labelId="selectLockboxLabel"
            {...register("LockBoxID")}
            value={getValues("LockBoxID")}
            defaultValue={lockboxes[0]?.lockBoxID}
            onChange={(event, _) =>
              setValue("LockBoxID", Number(event.target.value))
            }
          >
            {lockboxes.map((lockbox) => (
              <MenuItem value={lockbox.lockBoxID}>{lockbox.lockBoxID}</MenuItem>
            ))}
          </Select>
          </CustomToolTips>

          <Button
            className="max-w-xs self-center"
            type="submit"
            variant="outlined"
          >
            Legg til båt
          </Button>
        </Box>
      </form>
      <CustomSnackBar
        severity={snackbarSeverity}
        message={snackbarMessage}
        open={snackbarOpen}
        handleClose={closeSnackbar}
      />
    </Box>
  );
};

export default NewBoatForm;
