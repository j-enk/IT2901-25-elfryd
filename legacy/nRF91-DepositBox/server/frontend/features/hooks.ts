
import {TypedUseSelectorHook, useDispatch, useSelector} from 'react-redux';
import {AppDispatch, RootState, ThunkAppDispatch} from '../services/store';

export const useAppDispatch = () => useDispatch<AppDispatch>();
export const useAppThunkDispatch = () => useDispatch<ThunkAppDispatch>();
export const useAppSelector: TypedUseSelectorHook<RootState> = useSelector;