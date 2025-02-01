import FullCalendar, {
  DatesSetArg,
  DayCellContentArg,
  DayHeaderMountArg,
  EventClickArg,
  CalendarApi,
} from "@fullcalendar/react";
import interactionPlugin from "@fullcalendar/interaction";
import timeGridPlugin from "@fullcalendar/timegrid";
import { Dispatch, createRef, useEffect, useRef, useState } from "react";
import { useSelector } from "react-redux";
import CreateBookingComp from "./CreateBooking";
import {
  fetchBooking,
  resetBookingState,
  resetError,
} from "../../features/booking/bookingSlice";
import { useAppDispatch } from "../../features/hooks";
import BookingDetail from "./BookingDetail";
import { BookingPeriodType } from "../../typings/bookingPeriodType";
import { selectSelectedBoat } from "../../features/boat/boatSelector";
import Loading from "../loading/Loading";
import useBookings from "../../hooks/bookings/useBookings";
import { selectActiveUser } from "../../features/user/userSelector";
import getBookingPeriods from "../../hooks/getBookingPeriods";
import useRenderDesktopView from "../../hooks/shouldRenderDesktopView";

export const actionDispatch = (dispatch: Dispatch<any>) => ({
  resetState: () => dispatch(resetBookingState()),
  resetError: () => dispatch(resetError()),
  setBooking: (query: number) => dispatch(fetchBooking(query)),
});

const Calendar = () => {
  const [viewDates, setViewDates] = useState({ startDate: "", endDate: "" });
  const [newEvent, setNewEvent] = useState({ comment: "", start: "", end: "" });
  const [open, setOpen] = useState(false);
  const [dialogOpen, setDialogOpen] = useState(false);
  const { resetState, resetError, setBooking } = actionDispatch(
    useAppDispatch()
  );
  const boat = useSelector(selectSelectedBoat);
  const bookingPeriods = getBookingPeriods("", boat.boatID);
  const [currentBookingPeriod, setCurrentBookingPeriod] = useState<
    BookingPeriodType | undefined
  >();
  const activeUser = useSelector(selectActiveUser);
  const [initialView, setInitialView] = useState("timeGridWeek");
  const calendarRef = createRef<FullCalendar>();

  const { data, isLoading } = useBookings(
    viewDates.startDate,
    viewDates.endDate
  );

  const shouldRenderDesktop = useRenderDesktopView();

  useEffect(() => {
    if (shouldRenderDesktop) setInitialView("timeGridWeek");
    else setInitialView("timeGridDay");
  }, [shouldRenderDesktop]);

  useEffect(() => {
    const calendarApi: CalendarApi | void = calendarRef.current?.getApi();
    if (calendarApi) {
      calendarApi.changeView(initialView);
    }
  }, [initialView]);

  function buildEvents() {
    let events = [{}];

    if (isLoading || !data) return events;
    events = [
      ...data.map((booking) => {
        return !booking.isOfficial
          ? {
            id: booking.bookingID,
            title:
              "Booket av: " +
              booking.bookingOwner +
              " Tlf: " +
              booking.bookingOwnerNumber,
            start: booking.startTime,
            end: booking.endTime,
            backgroundColor:
              activeUser.name === booking.bookingOwner ? "" : "#C0C0C0",
            borderColor:
              activeUser.name === booking.bookingOwner ? "" : "#C0C0C0",
          }
          : {
            id: booking.bookingID,
            title: booking.comment,
            start: booking.startTime,
            end: booking.endTime,
            backgroundColor: "lightgreen",
            borderColor: "lightgreen",
          };
      }),
      ...data.map((booking) => {
        return booking.endTime == booking.chargingDone
          ? {}
          : {
            id: booking.bookingID,
            title: "Båten lader",
            start: booking.endTime,
            end: booking.chargingDone,
            backgroundColor: "lightblue",
            borderColor: "lightblue",
          };
      }),
    ];
    return events;
  }

  /**
   * Opens new booking form when calendar is clicked
   */
  function handleShow() {
    setOpen((wasOpen) => !wasOpen);
    resetError();
  }

  /**
   * Opens more information about booking when clicked on in calendar
   */
  function handleDialogShow() {
    setDialogOpen((wasOpen) => !wasOpen);
  }

  function handleDateSelection(arg: { startStr: string; endStr: string }) {
    setNewEvent({ ...newEvent, start: arg.startStr, end: arg.endStr });
    handleShow();
  }

  function handleDatesSet(arg: DatesSetArg) {
    setCurrentBookingPeriod(findCurrentBookingPeriod(arg.end));
    setViewDates({
      ...viewDates,
      startDate: arg.startStr,
      endDate: arg.endStr,
    });
    resetState();
  }

  function handleEventClick(arg: EventClickArg) {
    setBooking(parseInt(arg.event.id));
    handleDialogShow();
    resetState();
  }

  function handleDayCellDidMount(arg: DayCellContentArg) {
    if (!isDayInsideBookingPeriods(arg.date)) {
      arg.el.style.backgroundColor = "#F3F3F3";
    }
  }

  function handleDayHeaderDidMount(arg: DayHeaderMountArg) {
    if (!isDayInsideBookingPeriods(arg.date)) {
      arg.el.innerHTML = `
      <span>
        OBS! Booking stengt
        <br>
        ${arg.text}
      </span>
      `;
    }
  }

  function isDayInsideBookingPeriods(date: Date) {
    for (let bp of bookingPeriods.data!.items) {
      // Must be a better way to do this
      if (
        date.setHours(0, 0, 0, 0) >=
        new Date(bp.startDate).setHours(0, 0, 0, 0) &&
        date.setHours(0, 0, 0, 0) <= new Date(bp.endDate).setHours(0, 0, 0, 0)
      ) {
        return true;
      }
    }
    return false;
  }
  /**
   * Finds booking period for the week selected in calendar 
   */
  function findCurrentBookingPeriod(date: Date) {
    return bookingPeriods.data!.items.find((bp: BookingPeriodType) => {
      const startDate = new Date(bp.startDate);
      const endDate = new Date(bp.endDate);
      const currentEndDate = new Date(date);
      for (let i = 0; i <= 7; i++) {
        currentEndDate.setDate(currentEndDate.getDate() - 1);
        if (
          currentEndDate >= startDate &&
          currentEndDate <= endDate &&
          bp.boatID === boat.boatID
        ) {
          return true;
        }
      }
      return false;
    });
  }

  function timeString(input: number | undefined): string | undefined {
    if (!input && input != 0) return;
    if (input === 0) return "23:59:59";
    return input < 10 ? `0${input}:00` : `${input}:00`;
  }

  return (
    <div>
      <CreateBookingComp
        handleShow={handleShow}
        newEvent={newEvent}
        open={open}
      />
      {!bookingPeriods.isLoading && bookingPeriods.isFetched ? (
        <FullCalendar
          ref={calendarRef}
          key={boat.boatID}
          locale="nb"
          timeZone="Paris"
          events={buildEvents()}
          plugins={[timeGridPlugin, interactionPlugin]}
          eventClick={handleEventClick} // Opens booking information when booking is clicked
          select={handleDateSelection} // Opens new booking modal on calendar click
          initialView={initialView}
          buttonText={{ today: "I dag", week: "Uke", day: "Dag" }}
          headerToolbar={{
            left: "prev,next today",
            center: "title",
            right: "timeGridWeek,timeGridDay", // user can switch between the two
          }}
          selectLongPressDelay={150} // Specifies press delay when interacting with calendar on mobile devices
          contentHeight={"auto"}
          datesSet={handleDatesSet} // Sets the current week's booking period on week change
          dayCellDidMount={handleDayCellDidMount} // Colours days outside of booking period gray
          editable={false}
          selectable={true}
          allDaySlot={false}
          firstDay={1} // Sets first day of week to monday
          /**
           * Uses BookingCloses and BookingOpens to only render time cells
           * between the booking period time interval
           */
          slotMinTime={
            timeString(currentBookingPeriod?.bookingOpens) || "08:00"
          }
          slotMaxTime={
            timeString(currentBookingPeriod?.bookingCloses) || "22:00"
          }
          selectAllow={(arg) => isDayInsideBookingPeriods(arg.start)}  // Makes calendar uninteractable if week is outside booking period
          dayHeaderDidMount={handleDayHeaderDidMount} // Checks if week is in booking period and adds notice in header if it is 
        />
      ) : (
        <Loading />
      )}
      <BookingDetail handleShow={handleDialogShow} open={dialogOpen} />
    </div>
  );
};

export default Calendar;
