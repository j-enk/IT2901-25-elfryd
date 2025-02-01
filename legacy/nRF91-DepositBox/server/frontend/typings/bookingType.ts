export type BookingCalendarType = {
  bookingID: number;
  startTime: string;
  endTime: string;
  chargingDone: string;
  status: string;
  comment: string | null;
  bookingOwner: string;
  bookingOwnerNumber: string;
  isOfficial: boolean;
  boatName: string;
};

export type BookingVMType = {
  startTime: string;
  endTime: string;
  status: string;
  comment: string | null;
  userID: string;
  boatID: number;
  isOfficial: boolean;
};

export type UpcomingBookingType = {
  bookingID: number;
  bookingOwner: string;
};

export type BookingReadableType = {
  bookingID: number;
  startTime: string;
  endTime: string;
  chargingDone: string;
  keyTakenTime: string | null;
  keyReturnedTime: string | null;
  status: string;
  comment: string | null;
  bookingOwnerId: string;
  bookingOwner: string;
  bookingOwnerPhone: string;
  bookingOwnerEmail: string;
  bookedBoat: string;
  isOfficial: boolean;
};
