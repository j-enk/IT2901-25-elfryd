import BookingDetail from "../../../components/Bookings/BookingDetail";
import { Provider } from "react-redux";
import { store } from "../../../services/store";
import { QueryClient, QueryClientProvider } from "react-query";
import { BookingReadableType } from "../../../typings/bookingType";
import { BoatType } from "../../../typings/boatType";
import { UserType } from "../../../typings/userType";

const admin: UserType = {
  id: "1",
  name: "Test Testesen",
  phone: "123456789",
  email: "test@test.com",
  roles: ["Admin"],
  isActive: true,
};

const member: UserType = {
  id: "2",
  name: "Test McTestern",
  phone: "12345678",
  email: "testern@test.com",
  roles: ["member"],
  isActive: true,
};

const booking: BookingReadableType = {
  bookedBoat: "Elfryd",
  bookingID: 1,
  bookingOwner: "Test Testesen",
  bookingOwnerEmail: "test@test.com",
  bookingOwnerId: "1",
  bookingOwnerPhone: "12345678",
  chargingDone: "2023-03-14T14:00:00",
  comment: "",
  endTime: "2023-03-14T12:00:00",
  keyReturnedTime: null,
  keyTakenTime: null,
  startTime: "2023-03-14T08:00:00",
  status: "P",
  isOfficial: false,
};

const userBooking: BookingReadableType = {
  bookedBoat: "Elfryd",
  bookingID: 2,
  bookingOwner: "Test McTestern",
  bookingOwnerEmail: "testern@test.com",
  bookingOwnerId: "2",
  bookingOwnerPhone: "12345678",
  chargingDone: "2023-03-14T14:00:00",
  comment: "",
  endTime: "2023-03-14T12:00:00",
  keyReturnedTime: null,
  keyTakenTime: null,
  startTime: "2023-03-14T08:00:00",
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

const setBookingState = (booking: BookingReadableType) => {
  store.getState().booking = {
    status: "idle",
    error: null,
    filterOnUser: false,
    activeBooking: booking,
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

const setBoatState = (boat: BoatType) => {
  store.getState().boat = {
    selectedBoat: boat,
  };
};

const queryClient = new QueryClient();

before(() => {
  // Add boat to state manually
  setBoatState(boat);

  // Test fails without this as code tries to reset state after manual mutation
  Cypress.on("uncaught:exception", (err, runnable) => {
    return false;
  });
});

describe("BookingDetail user info", () => {
  it("BookingDetail contains correct user info", () => {
    setUserState(member);
    setBookingState(userBooking);

    cy.mount(
      <Provider store={store}>
        <QueryClientProvider client={queryClient}>
          <BookingDetail open={true} handleShow={() => {}} />
        </QueryClientProvider>
      </Provider>
    );

    const parsedDate = (dateString: string) => {
      const date = new Date(dateString);
      return date.toLocaleString("nb-NO");
    };

    const parsedStatus = (status: string) => {
      switch (status) {
        case "D":
          return "Ferdig";
        case "P":
          return "Ikke startet";
        case "O":
          return "Startet";
        default:
          return "";
      }
    };

    //Booking info
    cy.get(".flex-row > :nth-child(1) > :nth-child(2)")
      .contains(parsedDate(userBooking.startTime))
      .should("exist");
    cy.get(".flex-row > :nth-child(1) > :nth-child(3)")
      .contains(parsedDate(userBooking.endTime))
      .should("exist");
    cy.get(".flex-row > :nth-child(1) > :nth-child(4)")
      .contains(parsedDate(userBooking.chargingDone))
      .should("exist");
    cy.get(".flex-row > :nth-child(1) > :nth-child(5)")
      .contains(parsedStatus(userBooking!.status))
      .should("exist");
    cy.get(".flex-row > :nth-child(1) > :nth-child(6)")
      .contains(userBooking.bookedBoat)
      .should("exist");

    //User info
    cy.get(".flex-row > :nth-child(2) > :nth-child(2)")
      .contains(userBooking.bookingOwner)
      .should("exist");
    cy.get(".flex-row > :nth-child(2) > :nth-child(3)")
      .contains(userBooking.bookingOwnerPhone)
      .should("exist");
    cy.get(".flex-row > :nth-child(2) > :nth-child(4)")
      .contains(userBooking.bookingOwnerEmail)
      .should("exist");
  });
});

describe("BookingDetail as admin", () => {
  it("Admin does not get warning on booking deletion which is theirs", () => {
    setUserState(admin);
    setBookingState(booking);

    cy.mount(
      <Provider store={store}>
        <QueryClientProvider client={queryClient}>
          <BookingDetail open={true} handleShow={() => {}} />
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
      }
    ).as("cancelBookingMutation");

    cy.get("button").contains("Kanseller booking").should("exist");
    cy.get("button").click().click();
    cy.wait("@cancelBookingMutation").then((intercept) => {
      const { status, boatID, userID } = intercept.request.body;
      expect(status).to.equal("C");
      expect(boatID).to.equal(boat.boatID);
      expect(userID).to.equal(admin.id);
    });
    cy.get(".text-sm").should("not.exist");
  });

  it("Admin gets warning on booking deletion which is not theirs", () => {
    setUserState(admin);
    setBookingState(userBooking);

    cy.mount(
      <Provider store={store}>
        <QueryClientProvider client={queryClient}>
          <BookingDetail open={true} handleShow={() => {}} />
        </QueryClientProvider>
      </Provider>
    );

    cy.get("button").contains("Kanseller booking").should("exist");
    cy.get("button").click();
    cy.get(".text-sm").should("exist");
  });
});

describe("BookingDetail as user", () => {
  it("Cancel button does not appear for regular user on other peoples booking", () => {
    setUserState(member);
    setBookingState(booking);

    cy.mount(
      <Provider store={store}>
        <QueryClientProvider client={queryClient}>
          <BookingDetail open={true} handleShow={() => {}} />
        </QueryClientProvider>
      </Provider>
    );

    cy.get("button").should("not.exist");
  });

  it("Cancel button appears on users own booking", () => {
    setUserState(member);
    setBookingState(userBooking);

    cy.mount(
      <Provider store={store}>
        <QueryClientProvider client={queryClient}>
          <BookingDetail open={true} handleShow={() => {}} />
        </QueryClientProvider>
      </Provider>
    );

    cy.intercept(
      "PUT",
      `${process.env.API_URL_DEV}/Bookings/${userBooking.bookingID}`,
      {
        statusCode: 200,
        body: {
          success: true,
        },
      }
    ).as("cancelBookingMutation");

    cy.get("button").contains("Kanseller booking").should("exist");
    cy.get("button").click().click();
    cy.wait("@cancelBookingMutation").then((intercept) => {
      const { status, boatID, userID } = intercept.request.body;
      expect(status).to.equal("C");
      expect(boatID).to.equal(boat.boatID);
      expect(userID).to.equal(member.id);
    });
    cy.get(".text-sm").should("not.exist");
  });
});
