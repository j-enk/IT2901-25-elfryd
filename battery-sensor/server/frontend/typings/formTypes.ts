export type NewUserFormType = {
  Name: string;
  Email: string;
  Phone: string;
  Role: string;
  isActive: boolean;
};

export type EditUserFormType = {
  Name: string;
  Email: string;
  Phone: string;
  Role: string;
  id: string;
  isActive: boolean;
  bookingIDs: string[];
};

export type NewBookingFormType = {
  from: string;
  to: string;
  comment: string;
  isOfficial: boolean;
};

export type NewBookingPeriodFormType = {
  Name: string;
  StartDate: string;
  EndDate: string;
  BookingOpens: number;
  BookingCloses: number;
  BoatID: number;
};

export type EditBoatFormType = {
  BoatID: number;
  Name: string;
  ChargingTime: number;
  AdvanceBookingLimit: number;
  MaxBookingLimit: number;
  LockBoxID: number;
};

export type NewBoatFormType = {
  Name: string;
  ChargingTime: number;
  AdvanceBookingLimit: number;
  MaxBookingLimit: number;
  LockBoxID: number;
};
