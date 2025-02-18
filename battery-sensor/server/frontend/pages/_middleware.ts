import { NextResponse } from "next/server";
import type { NextRequest } from "next/server";
import jwt_decode from "jwt-decode";

export const middleware = async (request: NextRequest) => {
  type DecodedToken = {
    [key: string]: string;
  };

  let cookie = request.cookies["token"];
  let decodedToken = null;

  try {
    decodedToken = jwt_decode<DecodedToken>(cookie);
  } catch {}

  if (!decodedToken && request.nextUrl.pathname != "/") {
    return NextResponse.redirect(new URL("/", request.url));
  }

  if (
    decodedToken &&
    decodedToken["isActive"].includes("False") &&
    request.nextUrl.pathname != "/deactivated"
  ) {
    return NextResponse.redirect(new URL("/deactivated", request.url));
  }

  if (cookie && request.nextUrl.pathname == "/") {
    return NextResponse.redirect(new URL("/booking", request.url));
  }

  if (request.nextUrl.pathname.includes("/admin") && decodedToken) {
    if (decodedToken["roles"].includes("Admin")) {
      return NextResponse.next();
    } else {
      return NextResponse.redirect(new URL("/booking", request.url));
    }
  }

  return NextResponse.next();
};
