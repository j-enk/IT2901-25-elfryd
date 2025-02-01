import {
  Box,
  TextField,
  Button,
  Input,
  Select,
  MenuItem,
  SelectChangeEvent,
  InputLabel,
} from "@mui/material";
import { useForm } from "react-hook-form";
import { useSelector } from "react-redux";
import { selectSelectedBoat } from "../../../../features/boat/boatSelector";
import useNewBookingPeriodMutation from "../../../../hooks/newBookingPeriodMutation";
import { NewBookingPeriodFormType } from "../../../../typings/formTypes";
import { useEffect, useState } from "react";
import CustomSnackBar from "../../../snackbar/CustomSnackbar";
import useBoats from "../../../../hooks/boats/useBoats";
import Loading from "../../../loading/Loading";
import useCustomSnackbar from "../../../snackbar/useCustomSnackbar";
import CustomToolTips from "../../CustomToolTips";

export type ErrorReturnType = {
  response: {
    data: string;
  };
};

const NewBookingPeriodForm = () => {
  const { data: boats, isLoading: isBoatsLoading } = useBoats("", 1, 999);
  const globalSelectedBoat = useSelector(selectSelectedBoat);

  const [selectedBoat, setSelectedBoat] = useState(
    globalSelectedBoat.boatID > 0 ? globalSelectedBoat.boatID : 0
  );
  const { register, handleSubmit, reset } = useForm<NewBookingPeriodFormType>();
  const { mutate, isSuccess, isError } = useNewBookingPeriodMutation();

  const {
    snackbarOpen,
    snackbarMessage,
    snackbarSeverity,
    openSnackbar,
    closeSnackbar,
  } = useCustomSnackbar();

  function timeString(input: number): string {
    return input < 10 ? `0${input}:00` : `${input}:00`;
  }

  useEffect(() => {
    if (isSuccess) {
      openSnackbar("Bookingperioden ble lagt til", "success");
    }
  }, [isSuccess]);

  useEffect(() => {
    if (isError) {
      openSnackbar("Kunne ikke legge til bookingperiode", "error");
    }
  }, [isError]);

  function handleBoatChange(event: SelectChangeEvent<number>) {
    setSelectedBoat(event.target.value as number);
  }

  const onSubmit = (data: NewBookingPeriodFormType) => {
    mutate(data);
    reset();
  };

  if (!boats || isBoatsLoading)
    return (
      <Box width="100%" display="flex" justifyContent="center">
        <Loading />
        <CustomSnackBar
          severity={snackbarSeverity}
          message={snackbarMessage}
          open={snackbarOpen}
          handleClose={closeSnackbar}
        />
      </Box>
    );

  const dateInputStyle = {
    borderRadius: "4px",
    marginBottom: "0",
    fontSize: "1rem",
    marginTop: "2px",
    padding: "1rem",
    borderColor: "#C4C4C4",
  };

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
          <CustomToolTips
            title="Tittel"
            description="Navn på perioden, for eksempel 'Sesong 2023'"
          >
            <TextField
              fullWidth={true}
              label="Tittel"
              {...register("Name")}
              required
            />
          </CustomToolTips>
          <InputLabel id="label_boat_selector">Velg båt</InputLabel>
          <CustomToolTips title="Båtnavn" description="Om det er flere båter i systemet kan du velge mellom de her, om ikke trenger du ikke endre denne">
          <Select
            labelId="label_boat_selector"
            fullWidth={true}
            id="boat_selector"
            value={selectedBoat}
            {...register("BoatID", {
              onChange: (e) => {
                handleBoatChange(e);
              },
            })}
          >
            {boats.items.map((boat) => (
              <MenuItem key={boat.boatID} value={boat.boatID}>
                {boat.name}
              </MenuItem>
            ))}
          </Select>
            </CustomToolTips>
          <InputLabel>Velg startdato</InputLabel>
          <CustomToolTips
            title="Startdato"
            description="Datoen når bookingperioden starter"
          >
            <input
              className="input w-full mb-5 border border-gray-300"
              style={dateInputStyle}
              required
              type="date"
              {...register("StartDate")}
            />
          </CustomToolTips>
          <InputLabel>Velg sluttdato</InputLabel>
          <CustomToolTips
            title="Sluttdato"
            description="Datoen når bookingperioden avslutten"
          >
            <input
              className="input w-full mb-5 border border-gray-300"
              style={dateInputStyle}
              required
              type="date"
              {...register("EndDate")}
            />
          </CustomToolTips>
          <InputLabel id="opening-hour-label">Velg åpningstid</InputLabel>
          <CustomToolTips
            title="Åpningstid"
            description="Her velger du åpningstid for båten gjennom perioden, om du velger 08:00 vil åpningstiden være 08:00 hele perioden"
          >
            <Select
              labelId="opening-hour-label"
              id="opening-hour-select"
              defaultValue={8}
              fullWidth={true}
              MenuProps={{
                PaperProps: {
                  style: {
                    width: "50px",
                    height: "200px",
                    overflowX: "scroll",
                  },
                },
              }}
              {...register("BookingOpens")}
            >
              {[...Array(24)].map((_, index) => (
                <MenuItem key={index} value={index}>
                  {timeString(index)}
                </MenuItem>
              ))}
            </Select>
          </CustomToolTips>
          <InputLabel id="closing-hour-label">Velg stengetid</InputLabel>
          <CustomToolTips
            title="Stengetid"
            description="Her velger du stengetid for båten gjennom perioden, om du velger 21:00 vil stengetiden være 21:00 hele perioden"
          >
            <Select
              labelId="closing-hour-label"
              id="closing-hour-select"
              defaultValue={22}
              fullWidth={true}
              MenuProps={{
                PaperProps: {
                  style: {
                    width: "50px",
                    height: "200px",
                    overflowX: "scroll",
                  },
                },
              }}
              {...register("BookingCloses")}
            >
              {[...Array(24)].map((_, index) => (
                <MenuItem key={index} value={index}>
                  {timeString(index)}
                </MenuItem>
              ))}
            </Select>
          </CustomToolTips>
          <Button
            className="max-w-xs self-center"
            type="submit"
            variant="outlined"
          >
            Legg til bookingperiode
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

export default NewBookingPeriodForm;
