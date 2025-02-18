import EditBooking from "../../../components/Bookings/editBooking";
import { Provider } from "react-redux";
import { store } from "../../../services/store";
import { BookingReadableType } from "../../../typings/bookingType";
import { BoatType } from "../../../typings/boatType";
import { QueryClientProvider } from "react-query";
import { queryClient } from "../../../pages/_app";
import { UserType } from "../../../typings/userType";

const booking: BookingReadableType = {
  bookedBoat: "Elfryd",
  bookingID: 0,
  bookingOwner: "Test McTestern",
  bookingOwnerEmail: "testern@test.com",
  bookingOwnerId: "2",
  bookingOwnerPhone: "12345678",
  chargingDone: "2023-03-14T14:00",
  comment: "",
  endTime: "2023-03-14T12:00",
  keyReturnedTime: null,
  keyTakenTime: null,
  startTime: "2023-03-14T08:00",
  status: "P",
  isOfficial: false,
};

const boat: BoatType = {
  boatID: 1,
  name: "Elfryd",
  chargingTime: 50,
  advanceBookingLimit: 90,
  maxBookingLimit: 8,
  lockBoxID: 1,
};

const user: UserType = {
  id: "2",
  name: "Test McTestern",
  phone: "12345678",
  email: "testern@test.com",
  roles: ["member"],
  isActive: true,
};

const setBoatState = (boat: BoatType) => {
  store.getState().boat = {
    selectedBoat: boat,
  };
};

const setUserState = (user: UserType) => {
  store.getState().user = {
    activeUser: user,
    isLoggedIn: false,
    status: "idle",
    error: null,
  };
};

const setBookingState = (booking: BookingReadableType) => {
  const bookingState = store.getState().booking || {
    bookings: [],
    upcomingBooking: {
      bookingID: 0,
      bookingOwner: "",
    },
    status: "idle",
    error: null,
    filterOnUser: false,
    userHasBookedStatus: "idle",
    userHasBookedError: null,
    activeBooking: null,
  };

  store.getState().booking = {
    ...bookingState,
    activeBooking: booking,
  };
};

beforeEach(() => {
  // Test fails without this as code tries to reset state after manual mutation
  setBoatState(boat);
  setUserState(user);
  setBookingState(booking);

  Cypress.on("uncaught:exception", (err, runnable) => {
    return false;
  });
});

describe("Renders", () => {
  it("Renders component correctly", () => {
    // Set up test data

    const handleShow = cy.stub();
    const handleEdit = cy.stub();

    setBookingState(booking);

    // Mount the component
    cy.mount(
      <Provider store={store}>
        <EditBooking
          handleEdit={handleEdit}
          handleShow={handleShow}
          startTime={booking.startTime}
          endTime={booking.endTime}
        ></EditBooking>
      </Provider>
    );

    // Fill in form fields and submit form
    cy.get('input[type="datetime-local"]')
      .first()
      .should(($input) => {
        expect($input).to.have.value(booking.startTime);
      });
    cy.get('input[type="datetime-local"]')
      .eq(1)
      .should(($input) => {
        expect($input).to.have.value(booking.endTime);
      });
    cy.get("button").contains("Lagre booking");
  });
});

describe("Edit", () => {
  it("Saves edit succesfully", () => {
    // Stub the editBooking mutation
    const handleShow = cy.stub();
    const handleEdit = cy.stub();

    // Mount the component
    cy.mount(
      <Provider store={store}>
        <QueryClientProvider client={queryClient}>
          <EditBooking
            handleEdit={handleEdit}
            handleShow={handleShow}
            startTime={booking.startTime}
            endTime={booking.endTime}
          ></EditBooking>
        </QueryClientProvider>
      </Provider>
    );

    cy.intercept(
      "PUT",
      `${process.env.API_URL_DEV}/Bookings/${booking.bookingID}`,
      {
        statusCode: 200,
        body: {
          success: true,
        },
        error: false,
      }
    ).as("editBookingMutation");

    // Fill in form fields and submit form
    cy.get('input[type="datetime-local"]')
      .first()
      .clear()
      .type("2023-03-15T11:00");
    cy.get('input[type="datetime-local"]')
      .eq(1)
      .clear()
      .type("2023-03-15T12:00");
    cy.get("button").contains("Lagre booking").click().click();

    cy.wait("@editBookingMutation").then((intercept) => {
      const { startTime, endTime } = intercept.request.body;
      expect(startTime).to.equal("2023-03-15T11:00");
      expect(endTime).to.equal("2023-03-15T12:00");
      expect(handleEdit).to.be.calledOnce;
      expect(handleShow).to.be.calledOnce;
    });
  });
});
